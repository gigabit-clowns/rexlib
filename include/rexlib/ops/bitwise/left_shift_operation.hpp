// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

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
	binary_elementwise_shape_rule,
	binary_homogeneous_rule<integral_type_domain>
);

} // namespace ops
} // namespace rexlib
