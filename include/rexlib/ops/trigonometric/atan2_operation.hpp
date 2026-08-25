// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/basic_operation.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/policies/elementwise_operation_shape_policy.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @brief Compute the arc tangent of two arrays, by quadrant.
 *
 * The operands are the ordinate first and the abscissa second,
 * matching std::atan2 rather than reading left to right.
 */
REXLIB_DECLARE_OPERATION(
	atan2,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("y", "x"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<floating_point_type_domain>
);

} // namespace ops
} // namespace rexlib
