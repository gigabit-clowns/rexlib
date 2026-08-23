// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/basic_operation.hpp>
#include <xmipp4/ops/ops_component.hpp>
#include <xmipp4/ops/policies/elementwise_operation_shape_policy.hpp>
#include <xmipp4/ops/rules/operand_type_rules.hpp>

namespace xmipp4
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
REX_DECLARE_OPERATION(
	sign,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_homogeneous_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace xmipp4
