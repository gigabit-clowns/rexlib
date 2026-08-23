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
 * @brief Shift the bits of an array leftwards.
 *
 * Only integers are admitted. Under a homogeneous rule the count
 * carries the operand type, and a boolean or character shift
 * count is not a quantity that means anything.
 */
REX_DECLARE_OPERATION(
	left_shift,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value", "count"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<integral_type_domain>
);

} // namespace ops
} // namespace rex
