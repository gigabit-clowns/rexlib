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
 * @brief Shift the bits of an array leftwards.
 *
 * Only integers are admitted. Under a homogeneous rule the count
 * carries the operand type, and a boolean or character shift
 * count is not a quantity that means anything.
 */
REXLIB_DECLARE_OPERATION(
	left_shift,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value", "count"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<integral_type_domain>
);

} // namespace ops
} // namespace rexlib
