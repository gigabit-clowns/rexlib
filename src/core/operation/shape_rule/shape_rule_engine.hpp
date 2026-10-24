// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "shape_rule_resolution.hpp"

#include <rexlib/core/operation/operand_specification.hpp>
#include <rexlib/core/operation/operation_descriptor.hpp>
#include <rexlib/core/operation/shape_rule/shape_rule_descriptor.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>

namespace rexlib
{

/**
 * @defgroup shape_rule_engine Shape rule engine
 *
 * Runtime interpretation of a shape rule, lowered to a descriptor.
 *
 * Every function here throws std::invalid_argument when the operands or
 * the values of the operation instance are not admissible, and
 * std::out_of_range when an axis is named beyond the rank of the inputs.
 *
 * @{
 */

/**
 * @brief Check the values an operation instance supplies.
 *
 * @param descriptor Description of the operation, used to name it.
 * @param rule The rule.
 * @param named_axes The axes the core axes lie along, under named
 * placement.
 * @param operation_extents The extents the operation supplies, in the order
 * their variables are declared.
 */
void check_shape_rule_instance(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	span<const std::size_t> named_axes,
	span<const std::size_t> operation_extents
);

/**
 * @brief Resolve a rule against the shapes of the inputs.
 *
 * @param descriptor Description of the operation, used to name it and its
 * operands.
 * @param rule The rule.
 * @param named_axes The axes the core axes lie along, under named
 * placement.
 * @param operation_extents The extents the operation supplies.
 * @param output_count The number of outputs.
 * @param inputs The inputs, one per input core.
 * @return shape_rule_resolution The batch shape, the value of every extent
 * variable and the extents of the run.
 */
shape_rule_resolution resolve_shape_rule(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	span<const std::size_t> named_axes,
	span<const std::size_t> operation_extents,
	std::size_t output_count,
	span<const operand_specification> inputs
);

/**
 * @brief Compute the shape of an output from a resolution.
 *
 * @param rule The rule.
 * @param resolution The resolution of the rule.
 * @param named_axes The axes the core axes lie along, under named
 * placement.
 * @param keep_dimensions Whether an output without core axes keeps the
 * named ones, with an extent of one.
 * @param output Index of the output.
 * @return operand_specification::shape_type The shape.
 */
operand_specification::shape_type make_output_shape(
	const shape_rule_descriptor &rule,
	const shape_rule_resolution &resolution,
	span<const std::size_t> named_axes,
	bool keep_dimensions,
	std::size_t output
);

/**
 * @brief Check the shapes of outputs supplied in advance.
 *
 * @param descriptor Description of the operation, used to name it and its
 * operands.
 * @param rule The rule.
 * @param resolution The resolution of the rule.
 * @param named_axes The axes the core axes lie along, under named
 * placement.
 * @param keep_dimensions Whether an output without core axes keeps the
 * named ones.
 * @param user_outputs The outputs supplied in advance.
 */
void check_output_shapes(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	const shape_rule_resolution &resolution,
	span<const std::size_t> named_axes,
	bool keep_dimensions,
	span<const operand_specification> user_outputs
);

/** @} */

} // namespace rexlib
