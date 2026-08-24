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
 * @brief Round each element of the input array to the nearest.
 *
 * Ties are resolved towards the even neighbour, as IEEE 754
 * requires, rather than away from zero.
 */
REXLIB_DECLARE_OPERATION(
	round,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_homogeneous_rule<floating_point_type_domain>
);

} // namespace ops
} // namespace rex
