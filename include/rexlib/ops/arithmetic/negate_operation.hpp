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
 * @brief Negate each element of the input array into the output array.
 *
 * Only types that can represent a negative value are admitted, which rules
 * out booleans, characters and unsigned integers.
 */
REXLIB_DECLARE_OPERATION(
	negate,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	unary_elementwise_shape_rule,
	unary_homogeneous_rule<signed_arithmetic_type_domain>
);

} // namespace ops
} // namespace rexlib
