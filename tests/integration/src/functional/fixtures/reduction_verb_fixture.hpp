// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "cpu_execution_context_fixture.hpp"
#include "verb_type_sweep.hpp"

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/creation.hpp>

#include <rex/core/hardware/memory_resource_affinity.hpp>
#include <rex/core/ndarray/const_array_ref.hpp>
#include <rex/core/numerical/numerical_type.hpp>
#include <rex/core/numerical/numerical_type_traits.hpp>
#include <rex/core/numerical/scalar_value.hpp>

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace rex
{
namespace test
{

/**
 * @brief Fixture driving a reduction verb across every numerical type.
 *
 * Sweeps all of numerical_type, running the verb and comparing against a
 * reference for the types the operation's rule admits, and asserting a
 * rejection for the types it does not. Both halves are decided by the rule,
 * so a test states what the operation computes and never restates which
 * types it accepts.
 *
 * The input is filled with a single value, so the answer depends only on
 * that value and on how many elements reach each output. A reference is
 * therefore a function of those two things, which is enough to describe
 * every operation in the family: a sum multiplies, an average returns the
 * value untouched, an extremum likewise, a count answers with the count.
 */
class reduction_verb_fixture : public cpu_execution_context_fixture
{
protected:
	/**
	 * @brief The shape every reduction verb has.
	 *
	 * Spelled out rather than deduced because each verb is an overload set,
	 * carrying a reduce-these-axes overload alongside a reduce-everything
	 * one. Naming the signature is what picks the former.
	 */
	using verb_type = array (*)(
		const_array_ref,
		span<const std::ptrdiff_t>,
		bool,
		const execution_context&,
		array*
	);

	/**
	 * @brief Extents of the input the drivers build.
	 *
	 * Three axes of different extents, so that reducing over one of them
	 * cannot be confused with reducing over another, and so that both
	 * orientations of the loop are exercised: reducing the last axis walks
	 * the reduced elements contiguously, reducing the first walks the
	 * surviving ones contiguously.
	 *
	 * @return std::vector<std::size_t> The extents.
	 */
	static std::vector<std::size_t> input_extents()
	{
		return { 2, 3, 4 };
	}

	/**
	 * @brief Build a constant operand of a given element type.
	 *
	 * @tparam T The element type.
	 * @param extents The extents of the operand.
	 * @param value The value every element takes.
	 * @return array The operand.
	 */
	template <typename T>
	array make_operand(
		std::vector<std::size_t> extents,
		element_value value
	) const
	{
		return full(
			make_descriptor(std::move(extents), numerical_type_of<T>::value),
			memory_resource_affinity::device,
			scalar_value(value.as<T>()),
			context
		);
	}

	/**
	 * @brief Build an operand holding a given sequence of values.
	 *
	 * A constant operand cannot tell an extremum from an average or from the
	 * first element it met, so the cases that pin what a fold actually
	 * chooses need an operand whose elements differ.
	 *
	 * @tparam T The element type.
	 * @param extents The extents of the operand.
	 * @param values One value per element, in memory order. Each carries an
	 * imaginary part, so one list serves a real and a complex operand alike.
	 * @return array The operand.
	 */
	template <typename T>
	array make_sequence_operand(
		std::vector<std::size_t> extents,
		const std::vector<element_value> &values
	) const
	{
		auto result = empty(
			make_descriptor(std::move(extents), numerical_type_of<T>::value),
			memory_resource_affinity::device,
			context
		);

		auto *storage = result.get_storage();
		REQUIRE( storage != nullptr );
		auto *data = static_cast<T*>(storage->get_host_ptr());
		REQUIRE( data != nullptr );

		for (std::size_t i = 0; i < values.size(); ++i)
		{
			data[i] = values[i].as<T>();
		}

		return result;
	}

	/**
	 * @brief Check an output element by element.
	 *
	 * @tparam U The element type the output is expected to have.
	 * @param result The output array.
	 * @param extents The extents the output is expected to have.
	 * @param expected One value per element, in memory order.
	 */
	template <typename U>
	void check_values(
		const array &result,
		const std::vector<std::size_t> &extents,
		const std::vector<element_value> &expected
	) const
	{
		const auto data_type = numerical_type_of<U>::value;
		CHECK( result.get_descriptor() == make_descriptor(extents, data_type) );

		const auto values = read_host<U>(result, expected.size());
		for (std::size_t i = 0; i < expected.size(); ++i)
		{
			INFO( "element " << i );
			CHECK( values[i] == expected[i].as<U>() );
		}
	}

	/**
	 * @brief The shape a reduction over these axes leaves behind.
	 *
	 * @param extents The extents of the input.
	 * @param axes The axes being reduced, ascending and non negative.
	 * @param keep_dimensions Whether the reduced axes are kept at one.
	 * @return std::vector<std::size_t> The expected output extents.
	 */
	static std::vector<std::size_t> output_extents(
		const std::vector<std::size_t> &extents,
		const std::vector<std::ptrdiff_t> &axes,
		bool keep_dimensions
	)
	{
		std::vector<std::size_t> result;
		for (std::size_t i = 0; i < extents.size(); ++i)
		{
			const auto reduced = std::find(
				axes.cbegin(),
				axes.cend(),
				static_cast<std::ptrdiff_t>(i)
			) != axes.cend();

			if (!reduced)
			{
				result.push_back(extents[i]);
			}
			else if (keep_dimensions)
			{
				result.push_back(1);
			}
		}

		return result;
	}

	/**
	 * @brief The number of elements reaching each output element.
	 *
	 * @param extents The extents of the input.
	 * @param axes The axes being reduced.
	 * @return std::size_t The product of the reduced extents.
	 */
	static std::size_t reduction_count(
		const std::vector<std::size_t> &extents,
		const std::vector<std::ptrdiff_t> &axes
	)
	{
		std::size_t result = 1;
		for (const auto axis : axes)
		{
			result *= extents[static_cast<std::size_t>(axis)];
		}
		return result;
	}

	/**
	 * @brief Check a one input reduction verb across every numerical type.
	 *
	 * @tparam Op The operation the verb dispatches.
	 * @tparam Restriction Domain this case is confined to, for a property
	 * that only holds over part of the operation's own domain.
	 * @param verb The verb under test.
	 * @param axes The axes to reduce, ascending and non negative.
	 * @param keep_dimensions Whether the reduced axes are kept at one.
	 * @param x The value every element of the input takes.
	 * @param reference Called with the widened input and the number of
	 * elements folded into each output, giving the expected result. Only
	 * instantiated for the types the operation admits, so it may use
	 * operators the rejected types do not have.
	 * @param mode How closely the result must match.
	 */
	template <
		typename Op,
		typename Restriction = any_type_domain,
		typename Reference
	>
	void check_reduction(
		verb_type verb,
		std::vector<std::ptrdiff_t> axes,
		bool keep_dimensions,
		element_value x,
		Reference &&reference,
		comparison_mode mode = comparison_mode::exact
	) const
	{
		for (const auto type : all_numerical_types())
		{
			INFO( "element type " << type );
			dispatch_numerical_types(
				[&](auto tag)
				{
					using pivot_type = typename decltype(tag)::type;
					this->template run_reduction<Op, pivot_type>(
						detail::select_case<Op, Restriction, pivot_type>(),
						verb,
						axes,
						keep_dimensions,
						x,
						reference,
						mode
					);
				},
				type
			);
		}
	}

	/**
	 * @brief Check the output of a reduction against an expected value.
	 *
	 * @tparam U The element type the output is expected to have.
	 * @param result The output array.
	 * @param extents The extents the output is expected to have.
	 * @param expected The expected value, in its compute type.
	 * @param mode How closely it must match.
	 */
	template <typename U, typename Expected>
	void check_output(
		const array &result,
		const std::vector<std::size_t> &extents,
		const Expected &expected,
		comparison_mode mode
	) const
	{
		const auto data_type = numerical_type_of<U>::value;
		CHECK( result.get_descriptor() == make_descriptor(extents, data_type) );

		std::size_t count = 1;
		for (const auto extent : extents)
		{
			count *= extent;
		}

		const auto stored = detail::element_narrow<U>::apply(expected);
		for (const auto value : read_host<U>(result, count))
		{
			detail::element_checker<U>::apply(value, stored, mode);
		}
	}

private:
	// A type the case restricted itself away from costs nothing at all.
	template <typename Op, typename T, typename... Args>
	void run_reduction(detail::skip_case, Args&&...) const
	{
	}

	template <typename Op, typename T, typename Reference>
	void run_reduction(
		detail::run_case,
		verb_type verb,
		const std::vector<std::ptrdiff_t> &axes,
		bool keep_dimensions,
		element_value x,
		Reference &reference,
		comparison_mode mode
	) const
	{
		using operands = operand_element_types<Op, T>;
		using input_type = typename operands::template input_type<0>;
		using output_type = typename operands::template output_type<0>;

		const auto extents = input_extents();
		auto operand = make_operand<input_type>(extents, x);
		const const_array_ref operand_ref = operand;

		const auto result = verb(
			operand_ref,
			make_span(axes),
			keep_dimensions,
			context,
			nullptr
		);

		check_output<output_type>(
			result,
			output_extents(extents, axes, keep_dimensions),
			reference(widen(x.as<input_type>()), reduction_count(extents, axes)),
			mode
		);
	}

	template <typename Op, typename T, typename Reference>
	void run_reduction(
		detail::reject_case,
		verb_type verb,
		const std::vector<std::ptrdiff_t> &axes,
		bool keep_dimensions,
		element_value x,
		Reference&,
		comparison_mode
	) const
	{
		using operands = operand_element_types<Op, T>;
		using input_type = typename operands::template input_type<0>;

		auto operand = make_operand<input_type>(input_extents(), x);
		const const_array_ref operand_ref = operand;

		CHECK_THROWS_AS(
			verb(
				operand_ref,
				make_span(axes),
				keep_dimensions,
				context,
				nullptr
			),
			std::invalid_argument
		);
	}
};

} // namespace test
} // namespace rex
