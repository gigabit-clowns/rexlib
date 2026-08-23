// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/basic_operation.hpp>
#include <xmipp4/ops/ops_component.hpp>
#include <xmipp4/ops/policies/elementwise_operation_shape_policy.hpp>
#include <xmipp4/ops/rules/operand_type_rules.hpp>

namespace rex
{
namespace ops
{

/**
 * @brief Divide one by each element of the input array.
 *
 * Integral operands are not admitted: the reciprocal of every integer of
 * magnitude above one is zero, which is a trap rather than a result.
 */
REX_DECLARE_OPERATION(
	reciprocal,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_homogeneous_rule<inexact_type_domain>
);

} // namespace ops
} // namespace rex
