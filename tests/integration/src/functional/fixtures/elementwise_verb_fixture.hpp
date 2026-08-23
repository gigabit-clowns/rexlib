// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "cpu_execution_context_fixture.hpp"
#include "verb_type_sweep.hpp"

#include <catch2/catch_test_macros.hpp>

#include <xmipp4/functional/creation.hpp>

#include <xmipp4/core/hardware/memory_resource_affinity.hpp>
#include <xmipp4/core/ndarray/const_array_ref.hpp>
#include <xmipp4/core/numerical/numerical_type.hpp>
#include <xmipp4/core/numerical/numerical_type_traits.hpp>
#include <xmipp4/core/numerical/scalar_value.hpp>

#include <cstddef>
#include <stdexcept>

namespace rex
{
namespace test
{

/**
 * @brief Fixture driving an elementwise verb across every numerical type.
 *
 * Each driver sweeps all of numerical_type, running the verb and comparing
 * against a reference for the types the operation's rule admits, and
 * asserting a rejection for the types it does not. Both halves are decided
 * by the rule, so a test states what the operation computes and never
 * restates which types it accepts.
 */
class elementwise_verb_fixture : public cpu_execution_context_fixture
{
protected:
	/// Number of elements every operand in these drivers holds.
	static const std::size_t element_count = 6;

	/**
	 * @brief Build a constant operand of a given element type.
	 *
	 * @tparam T The element type.
	 * @param value The value every element takes.
	 * @return array The operand.
	 */
	template <typename T>
	array make_operand(element_value value) const
	{
		return full(
			make_descriptor({ 2, 3 }, numerical_type_of<T>::value),
			memory_resource_affinity::device,
			scalar_value(value.as<T>()),
			context
		);
	}

	/**
	 * @brief Check a one input verb across every numerical type.
	 *
	 * @tparam Op The operation the verb dispatches.
	 * @tparam Restriction Domain this case is confined to, for a property
	 * that only holds over part of the operation's own domain.
	 * @param verb The verb under test.
	 * @param x The value every element of the input takes.
	 * @param reference Called with the widened input, giving the expected
	 * result. Only instantiated for the types the operation admits, so it
	 * may use operators the rejected types do not have.
	 * @param mode How closely the result must match.
	 */
	template <
		typename Op,
		typename Restriction = any_type_domain,
		typename Verb,
		typename Reference
	>
	void check_unary(
		Verb &&verb,
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
					this->template run_unary<Op, pivot_type>(
						detail::select_case<Op, Restriction, pivot_type>(),
						verb,
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
	 * @brief Check a two input verb across every numerical type.
	 *
	 * @tparam Op The operation the verb dispatches.
	 * @tparam Restriction Domain this case is confined to.
	 * @param verb The verb under test.
	 * @param x The value every element of the first input takes.
	 * @param y The value every element of the second input takes.
	 * @param reference Called with both widened inputs.
	 * @param mode How closely the result must match.
	 */
	template <
		typename Op,
		typename Restriction = any_type_domain,
		typename Verb,
		typename Reference
	>
	void check_binary(
		Verb &&verb,
		element_value x,
		element_value y,
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
					this->template run_binary<Op, pivot_type>(
						detail::select_case<Op, Restriction, pivot_type>(),
						verb,
						x,
						y,
						reference,
						mode
					);
				},
				type
			);
		}
	}

	/**
	 * @brief Check a three input verb across every numerical type.
	 *
	 * @tparam Op The operation the verb dispatches.
	 * @tparam Restriction Domain this case is confined to.
	 * @param verb The verb under test.
	 * @param x The value every element of the first input takes.
	 * @param y The value every element of the second input takes.
	 * @param z The value every element of the third input takes.
	 * @param reference Called with all three widened inputs.
	 * @param mode How closely the result must match.
	 */
	template <
		typename Op,
		typename Restriction = any_type_domain,
		typename Verb,
		typename Reference
	>
	void check_ternary(
		Verb &&verb,
		element_value x,
		element_value y,
		element_value z,
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
					this->template run_ternary<Op, pivot_type>(
						detail::select_case<Op, Restriction, pivot_type>(),
						verb,
						x,
						y,
						z,
						reference,
						mode
					);
				},
				type
			);
		}
	}

	/**
	 * @brief Check a one input, two output verb across every type.
	 *
	 * @tparam Op The operation the verb dispatches.
	 * @tparam Restriction Domain this case is confined to.
	 * @param verb The verb under test, returning both results as a pair.
	 * @param x The value every element of the input takes.
	 * @param first_reference Called with the widened input, giving the
	 * expected first result.
	 * @param second_reference The same, for the second result.
	 * @param mode How closely the results must match.
	 */
	template <
		typename Op,
		typename Restriction = any_type_domain,
		typename Verb,
		typename FirstReference,
		typename SecondReference
	>
	void check_unary_pair(
		Verb &&verb,
		element_value x,
		FirstReference &&first_reference,
		SecondReference &&second_reference,
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
					this->template run_unary_pair<Op, pivot_type>(
						detail::select_case<Op, Restriction, pivot_type>(),
						verb,
						x,
						first_reference,
						second_reference,
						mode
					);
				},
				type
			);
		}
	}

	/**
	 * @brief Check a two input, two output verb across every type.
	 *
	 * @tparam Op The operation the verb dispatches.
	 * @tparam Restriction Domain this case is confined to.
	 * @param verb The verb under test, returning both results as a pair.
	 * @param x The value every element of the first input takes.
	 * @param y The value every element of the second input takes.
	 * @param first_reference Called with both widened inputs, giving the
	 * expected first result.
	 * @param second_reference The same, for the second result.
	 * @param mode How closely the results must match.
	 */
	template <
		typename Op,
		typename Restriction = any_type_domain,
		typename Verb,
		typename FirstReference,
		typename SecondReference
	>
	void check_binary_pair(
		Verb &&verb,
		element_value x,
		element_value y,
		FirstReference &&first_reference,
		SecondReference &&second_reference,
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
					this->template run_binary_pair<Op, pivot_type>(
						detail::select_case<Op, Restriction, pivot_type>(),
						verb,
						x,
						y,
						first_reference,
						second_reference,
						mode
					);
				},
				type
			);
		}
	}

	/**
	 * @brief Check one output of a verb against an expected value.
	 *
	 * @tparam U The element type the output is expected to have.
	 * @param result The output array.
	 * @param expected The expected value, in its compute type.
	 * @param mode How closely it must match.
	 */
	template <typename U, typename Expected>
	void check_output(
		const array &result,
		const Expected &expected,
		comparison_mode mode
	) const
	{
		const auto data_type = numerical_type_of<U>::value;
		CHECK( result.get_descriptor() == make_descriptor({ 2, 3 }, data_type) );

		const auto stored = detail::element_narrow<U>::apply(expected);
		for (const auto value : read_host<U>(result, element_count))
		{
			detail::element_checker<U>::apply(value, stored, mode);
		}
	}

private:
	// A type the case restricted itself away from costs nothing at all.
	template <typename Op, typename T, typename... Args>
	void run_unary(detail::skip_case, Args&&...) const
	{
	}

	template <typename Op, typename T, typename... Args>
	void run_binary(detail::skip_case, Args&&...) const
	{
	}

	template <typename Op, typename T, typename... Args>
	void run_ternary(detail::skip_case, Args&&...) const
	{
	}

	template <typename Op, typename T, typename... Args>
	void run_binary_pair(detail::skip_case, Args&&...) const
	{
	}

	template <typename Op, typename T, typename... Args>
	void run_unary_pair(detail::skip_case, Args&&...) const
	{
	}

	template <
		typename Op,
		typename T,
		typename Verb,
		typename FirstReference,
		typename SecondReference
	>
	void run_unary_pair(
		detail::run_case,
		Verb &verb,
		element_value x,
		FirstReference &first_reference,
		SecondReference &second_reference,
		comparison_mode mode
	) const
	{
		using operands = operand_element_types<Op, T>;
		using input_type = typename operands::template input_type<0>;
		using first_type = typename operands::template output_type<0>;
		using second_type = typename operands::template output_type<1>;

		auto operand = make_operand<input_type>(x);
		const const_array_ref operand_ref = operand;

		const auto results = verb(operand_ref, context, nullptr, nullptr);

		const auto widened = widen(x.as<input_type>());
		check_output<first_type>(
			results.first, first_reference(widened), mode
		);
		check_output<second_type>(
			results.second, second_reference(widened), mode
		);
	}

	template <
		typename Op,
		typename T,
		typename Verb,
		typename FirstReference,
		typename SecondReference
	>
	void run_unary_pair(
		detail::reject_case,
		Verb &verb,
		element_value x,
		FirstReference &,
		SecondReference &,
		comparison_mode
	) const
	{
		using operands = operand_element_types<Op, T>;
		using input_type = typename operands::template input_type<0>;

		auto operand = make_operand<input_type>(x);
		const const_array_ref operand_ref = operand;

		CHECK_THROWS_AS(
			verb(operand_ref, context, nullptr, nullptr),
			std::invalid_argument
		);
	}

	template <
		typename Op,
		typename T,
		typename Verb,
		typename FirstReference,
		typename SecondReference
	>
	void run_binary_pair(
		detail::run_case,
		Verb &verb,
		element_value x,
		element_value y,
		FirstReference &first_reference,
		SecondReference &second_reference,
		comparison_mode mode
	) const
	{
		using operands = operand_element_types<Op, T>;
		using left_type = typename operands::template input_type<0>;
		using right_type = typename operands::template input_type<1>;
		using first_type = typename operands::template output_type<0>;
		using second_type = typename operands::template output_type<1>;

		auto left = make_operand<left_type>(x);
		auto right = make_operand<right_type>(y);
		const const_array_ref left_ref = left;
		const const_array_ref right_ref = right;

		const auto results = verb(left_ref, right_ref, context, nullptr,
		                          nullptr);

		const auto widened_left = widen(x.as<left_type>());
		const auto widened_right = widen(y.as<right_type>());
		check_output<first_type>(
			results.first,
			first_reference(widened_left, widened_right),
			mode
		);
		check_output<second_type>(
			results.second,
			second_reference(widened_left, widened_right),
			mode
		);
	}

	template <
		typename Op,
		typename T,
		typename Verb,
		typename FirstReference,
		typename SecondReference
	>
	void run_binary_pair(
		detail::reject_case,
		Verb &verb,
		element_value x,
		element_value y,
		FirstReference &,
		SecondReference &,
		comparison_mode
	) const
	{
		using operands = operand_element_types<Op, T>;
		using left_type = typename operands::template input_type<0>;
		using right_type = typename operands::template input_type<1>;

		auto left = make_operand<left_type>(x);
		auto right = make_operand<right_type>(y);
		const const_array_ref left_ref = left;
		const const_array_ref right_ref = right;

		CHECK_THROWS_AS(
			verb(left_ref, right_ref, context, nullptr, nullptr),
			std::invalid_argument
		);
	}

	template <typename Op, typename T, typename Verb, typename Reference>
	void run_unary(
		detail::run_case,
		Verb &verb,
		element_value x,
		Reference &reference,
		comparison_mode mode
	) const
	{
		using operands = operand_element_types<Op, T>;
		using input_type = typename operands::template input_type<0>;
		using output_type = typename operands::template output_type<0>;

		auto operand = make_operand<input_type>(x);
		const const_array_ref operand_ref = operand;

		const auto result = verb(operand_ref, context, nullptr);
		check_output<output_type>(
			result,
			reference(widen(x.as<input_type>())),
			mode
		);
	}

	template <typename Op, typename T, typename Verb, typename Reference>
	void run_unary(
		detail::reject_case,
		Verb &verb,
		element_value x,
		Reference &,
		comparison_mode
	) const
	{
		using operands = operand_element_types<Op, T>;
		using input_type = typename operands::template input_type<0>;

		auto operand = make_operand<input_type>(x);
		const const_array_ref operand_ref = operand;

		CHECK_THROWS_AS(
			verb(operand_ref, context, nullptr),
			std::invalid_argument
		);
	}

	template <typename Op, typename T, typename Verb, typename Reference>
	void run_binary(
		detail::run_case,
		Verb &verb,
		element_value x,
		element_value y,
		Reference &reference,
		comparison_mode mode
	) const
	{
		using operands = operand_element_types<Op, T>;
		using left_type = typename operands::template input_type<0>;
		using right_type = typename operands::template input_type<1>;
		using output_type = typename operands::template output_type<0>;

		auto left = make_operand<left_type>(x);
		auto right = make_operand<right_type>(y);
		const const_array_ref left_ref = left;
		const const_array_ref right_ref = right;

		const auto result = verb(left_ref, right_ref, context, nullptr);
		check_output<output_type>(
			result,
			reference(widen(x.as<left_type>()), widen(y.as<right_type>())),
			mode
		);
	}

	template <typename Op, typename T, typename Verb, typename Reference>
	void run_binary(
		detail::reject_case,
		Verb &verb,
		element_value x,
		element_value y,
		Reference &,
		comparison_mode
	) const
	{
		using operands = operand_element_types<Op, T>;
		using left_type = typename operands::template input_type<0>;
		using right_type = typename operands::template input_type<1>;

		auto left = make_operand<left_type>(x);
		auto right = make_operand<right_type>(y);
		const const_array_ref left_ref = left;
		const const_array_ref right_ref = right;

		CHECK_THROWS_AS(
			verb(left_ref, right_ref, context, nullptr),
			std::invalid_argument
		);
	}

	template <typename Op, typename T, typename Verb, typename Reference>
	void run_ternary(
		detail::run_case,
		Verb &verb,
		element_value x,
		element_value y,
		element_value z,
		Reference &reference,
		comparison_mode mode
	) const
	{
		using operands = operand_element_types<Op, T>;
		using first_type = typename operands::template input_type<0>;
		using second_type = typename operands::template input_type<1>;
		using third_type = typename operands::template input_type<2>;
		using output_type = typename operands::template output_type<0>;

		auto first = make_operand<first_type>(x);
		auto second = make_operand<second_type>(y);
		auto third = make_operand<third_type>(z);
		const const_array_ref first_ref = first;
		const const_array_ref second_ref = second;
		const const_array_ref third_ref = third;

		const auto result = verb(
			first_ref, second_ref, third_ref, context, nullptr
		);
		check_output<output_type>(
			result,
			reference(
				widen(x.as<first_type>()),
				widen(y.as<second_type>()),
				widen(z.as<third_type>())
			),
			mode
		);
	}

	template <typename Op, typename T, typename Verb, typename Reference>
	void run_ternary(
		detail::reject_case,
		Verb &verb,
		element_value x,
		element_value y,
		element_value z,
		Reference &,
		comparison_mode
	) const
	{
		using operands = operand_element_types<Op, T>;
		using first_type = typename operands::template input_type<0>;
		using second_type = typename operands::template input_type<1>;
		using third_type = typename operands::template input_type<2>;

		auto first = make_operand<first_type>(x);
		auto second = make_operand<second_type>(y);
		auto third = make_operand<third_type>(z);
		const const_array_ref first_ref = first;
		const const_array_ref second_ref = second;
		const const_array_ref third_ref = third;

		CHECK_THROWS_AS(
			verb(first_ref, second_ref, third_ref, context, nullptr),
			std::invalid_argument
		);
	}
};

} // namespace test
} // namespace rex
