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
 * @brief Convert each element of the input array to degrees.
 */
REXLIB_DECLARE_OPERATION(
	degrees,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("radians"),
	elementwise_operation_shape_policy,
	unary_homogeneous_rule<floating_point_type_domain>
);

} // namespace ops
} // namespace rex
