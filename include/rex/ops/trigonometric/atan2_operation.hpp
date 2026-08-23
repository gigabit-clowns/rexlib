// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/basic_operation.hpp>
#include <rex/ops/ops_component.hpp>
#include <rex/ops/policies/elementwise_operation_shape_policy.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

namespace rex
{
namespace ops
{

/**
 * @brief Compute the arc tangent of two arrays, by quadrant.
 *
 * The operands are the ordinate first and the abscissa second,
 * matching std::atan2 rather than reading left to right.
 */
REX_DECLARE_OPERATION(
	atan2,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("y", "x"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<floating_point_type_domain>
);

} // namespace ops
} // namespace rex
