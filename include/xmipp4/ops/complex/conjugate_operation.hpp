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
 * @brief Conjugate each element of the input array.
 *
 * Real operands are admitted and left unchanged, so that code
 * generic over the element type does not need a special case.
 */
REX_DECLARE_OPERATION(
	conjugate,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_homogeneous_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace xmipp4
