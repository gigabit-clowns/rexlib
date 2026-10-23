// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/basic_operation.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/rules/operand_shape_rules.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @brief Shift the bits of an array rightwards.
 *
 * Only integers are admitted. Under a homogeneous rule the count
 * carries the operand type, and a boolean or character shift
 * count is not a quantity that means anything.
 */
REXLIB_DECLARE_OPERATION(
	right_shift,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value", "count"),
	binary_elementwise_shape_rule,
	binary_homogeneous_rule<integral_type_domain>
);

} // namespace ops
} // namespace rexlib
