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
 * @brief Extract the sign of each element of the input array.
 *
 * Complex operands yield the unit value with the same argument, which is
 * the closest analogue of a sign the complex plane has, and keeps the
 * result in the operand type.
 */
REXLIB_DECLARE_OPERATION(
	sign,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_homogeneous_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace rex
