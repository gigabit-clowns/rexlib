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
 * @brief Compute the cube root of each element of the input array.
 *
 * Unlike the square root this is defined for negative operands, and
 * unlike it there is no complex counterpart to fall back on.
 */
REX_DECLARE_OPERATION(
	cbrt,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_homogeneous_rule<floating_point_type_domain>
);

} // namespace ops
} // namespace rex
