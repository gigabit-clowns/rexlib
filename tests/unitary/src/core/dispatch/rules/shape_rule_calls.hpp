// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operand_names.hpp>
#include <rexlib/core/operation/contract/operand_specification.hpp>
#include <rexlib/core/operation/operation_descriptor.hpp>
#include <rexlib/core/dispatch/rules/shape_rule_descriptor_of.hpp>
#include <rexlib/core/dispatch/rules/shape_rule_interpreter.hpp>
#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <initializer_list>
#include <vector>

namespace rexlib
{
namespace test
{

using shape_type = operand_specification::shape_type;
using extent_list = std::vector<std::size_t>;

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
	const extent_list &named_axes = extent_list(),
	bool keep_dimensions = false,
	const extent_list &operation_extents = extent_list()
)
{
	const auto inputs = make_shaped_operands(input_shapes);
	std::vector<operand_specification> outputs(output_count);
	make_shape_rule_interpreter<Rule>().deduce(
		shape_rule_test_descriptor(),
		make_span(named_axes),
		keep_dimensions,
		make_span(operation_extents),
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
	const extent_list &named_axes = extent_list(),
	bool keep_dimensions = false,
	const extent_list &operation_extents = extent_list()
)
{
	return deduce_shapes<Rule>(
		1,
		input_shapes,
		named_axes,
		keep_dimensions,
		operation_extents
	)[0];
}

/**
 * @brief Ask a rule to accept outputs supplied in advance.
 */
template <typename Rule>
void accept_shapes(
	std::initializer_list<shape_type> output_shapes,
	std::initializer_list<shape_type> input_shapes,
	const extent_list &named_axes = extent_list(),
	bool keep_dimensions = false,
	const extent_list &operation_extents = extent_list()
)
{
	const auto outputs = make_shaped_operands(output_shapes);
	const auto inputs = make_shaped_operands(input_shapes);
	make_shape_rule_interpreter<Rule>().accept(
		shape_rule_test_descriptor(),
		make_span(named_axes),
		keep_dimensions,
		make_span(operation_extents),
		make_span(outputs),
		make_span(inputs)
	);
}

/**
 * @brief Ask a rule to check the values an operation instance supplies.
 */
template <typename Rule>
void check_shape_instance(
	const extent_list &named_axes,
	const extent_list &operation_extents = extent_list()
)
{
	make_shape_rule_interpreter<Rule>().check_instance(
		shape_rule_test_descriptor(),
		make_span(named_axes),
		make_span(operation_extents)
	);
}

} // namespace test
} // namespace rexlib
