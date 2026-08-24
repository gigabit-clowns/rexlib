// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/basic_operation.hpp>
#include <rex/ops/ops_component.hpp>
#include <rex/ops/policies/elementwise_operation_shape_policy.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

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
	elementwise_operation_shape_policy,
	unary_homogeneous_rule<signed_arithmetic_type_domain>
);

} // namespace ops
} // namespace rexlib
