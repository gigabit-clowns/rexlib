// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/operation/operand_names.hpp>
#include <rexlib/core/operation/operand_specification.hpp>
#include <rexlib/core/operation/operation_descriptor.hpp>
#include <rexlib/core/operation/shape_rule/extent_pivot_source.hpp>
#include <rexlib/core/operation/shape_rule/shape_rule_descriptor_of.hpp>
#include <rexlib/core/operation/shape_rule/shape_rule_interpreter.hpp>
#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <vector>

namespace rexlib
{
namespace test
{

using shape_type = operand_specification::shape_type;
using extent_list_type = std::vector<std::size_t>;

/**
 * @brief Description of an operation with two outputs and two inputs.
 *
 * Its operands are named "result", "other_result", "left" and "right", so
 * that a diagnostic can be checked for them.
 */
inline
const operation_descriptor& shape_rule_test_descriptor()
{
	static REXLIB_CONST_CONSTEXPR auto outputs =
		make_operand_names("result", "other_result");
	static REXLIB_CONST_CONSTEXPR auto inputs =
		make_operand_names("left", "right");
	static const operation_descriptor instance = make_operation_descriptor(
		"rexlib.test",
		"shaped",
		outputs,
		inputs
	);
	return instance;
}

inline
std::vector<operand_specification> make_shaped_operands(
	std::initializer_list<shape_type> shapes
)
{
	std::vector<operand_specification> result;
	for (const auto &shape : shapes)
	{
		result.emplace_back(shape, numerical_type::float32);
	}
	return result;
}

namespace detail
{

template <typename Pivots>
struct supplied_extents_of;

template <>
struct supplied_extents_of<type_list<>>
{
	static void append(extent_list_type&, const extent_list_type&, std::size_t&)
	{
	}
};

template <typename Head, typename... Tail>
struct supplied_extents_of<type_list<Head, Tail...>>
{
	static void append(
		extent_list_type &result,
		const extent_list_type &instance_extents,
		std::size_t &next
	)
	{
		append_one(
			result,
			instance_extents,
			next,
			std::integral_constant<extent_pivot_source, Head::source>()
		);
		supplied_extents_of<type_list<Tail...>>::append(
			result,
			instance_extents,
			next
		);
	}

private:
	static void append_one(
		extent_list_type&,
		const extent_list_type&,
		std::size_t&,
		std::integral_constant<extent_pivot_source, extent_pivot_source::inputs>
	)
	{
	}

	static void append_one(
		extent_list_type &result,
		const extent_list_type &instance_extents,
		std::size_t &next,
		std::integral_constant<
			extent_pivot_source,
			extent_pivot_source::operation
		>
	)
	{
		if (next < instance_extents.size())
		{
			result.push_back(instance_extents[next++]);
		}
	}

	static void append_one(
		extent_list_type &result,
		const extent_list_type&,
		std::size_t&,
		std::integral_constant<extent_pivot_source, extent_pivot_source::fixed>
	)
	{
		result.push_back(Head::accessor_type::get(0));
	}
};

} // namespace detail

/**
 * @brief Interleave the extents a rule fixes with those an instance
 * supplies, in the order the rule declares them.
 *
 * Instance extents the rule does not read are appended, so that a count
 * mismatch still reaches the interpreter.
 *
 * @tparam Rule The operand_shape_rule.
 * @param instance_extents The extents the operation instance supplies.
 * @return extent_list_type Every extent the operation supplies.
 */
template <typename Rule>
extent_list_type make_supplied_extents(const extent_list_type &instance_extents)
{
	extent_list_type result;
	std::size_t next = 0;
	detail::supplied_extents_of<typename Rule::pivot_list_type>::append(
		result,
		instance_extents,
		next
	);
	result.insert(
		result.end(),
		instance_extents.begin() + static_cast<std::ptrdiff_t>(next),
		instance_extents.end()
	);
	return result;
}

template <typename Rule>
shape_rule_interpreter make_shape_rule_interpreter()
{
	return shape_rule_interpreter(shape_rule_descriptor_of<Rule>::get());
}

/**
 * @brief Deduce the output shapes a rule gives a set of inputs.
 */
template <typename Rule>
std::vector<shape_type> deduce_shapes(
	std::size_t output_count,
	std::initializer_list<shape_type> input_shapes,
	const extent_list_type &named_axes = extent_list_type(),
	bool keep_dimensions = false,
	const extent_list_type &instance_extents = extent_list_type()
)
{
	const auto inputs = make_shaped_operands(input_shapes);
	std::vector<operand_specification> outputs(output_count);
	make_shape_rule_interpreter<Rule>().deduce(
		shape_rule_test_descriptor(),
		make_span(named_axes),
		keep_dimensions,
		make_span(make_supplied_extents<Rule>(instance_extents)),
		make_span(outputs),
		make_span(inputs)
	);

	std::vector<shape_type> result;
	for (const auto &output : outputs)
	{
		result.push_back(output.get_shape());
	}
	return result;
}

/**
 * @brief Deduce the shape a rule with one output gives a set of inputs.
 */
template <typename Rule>
shape_type deduce_shape(
	std::initializer_list<shape_type> input_shapes,
	const extent_list_type &named_axes = extent_list_type(),
	bool keep_dimensions = false,
	const extent_list_type &instance_extents = extent_list_type()
)
{
	return deduce_shapes<Rule>(
		1,
		input_shapes,
		named_axes,
		keep_dimensions,
		instance_extents
	)[0];
}

/**
 * @brief Ask a rule to accept outputs supplied in advance.
 */
template <typename Rule>
void accept_shapes(
	std::initializer_list<shape_type> output_shapes,
	std::initializer_list<shape_type> input_shapes,
	const extent_list_type &named_axes = extent_list_type(),
	bool keep_dimensions = false,
	const extent_list_type &instance_extents = extent_list_type()
)
{
	const auto outputs = make_shaped_operands(output_shapes);
	const auto inputs = make_shaped_operands(input_shapes);
	make_shape_rule_interpreter<Rule>().accept(
		shape_rule_test_descriptor(),
		make_span(named_axes),
		keep_dimensions,
		make_span(make_supplied_extents<Rule>(instance_extents)),
		make_span(outputs),
		make_span(inputs)
	);
}

/**
 * @brief Ask a rule to check the values an operation instance supplies.
 */
template <typename Rule>
void check_shape_instance(
	const extent_list_type &named_axes,
	const extent_list_type &instance_extents = extent_list_type()
)
{
	make_shape_rule_interpreter<Rule>().check_instance(
		shape_rule_test_descriptor(),
		make_span(named_axes),
		make_span(make_supplied_extents<Rule>(instance_extents))
	);
}

} // namespace test
} // namespace rexlib
