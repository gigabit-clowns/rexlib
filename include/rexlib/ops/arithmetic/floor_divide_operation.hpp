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
 * @brief Divide two input arrays, rounding the quotient downwards.
 *
 * The quotient is rounded towards negative infinity rather than towards
 * zero, so that it pairs with the remainder computed by modulo. Complex
 * operands are not admitted, having no ordering to round against.
 */
REXLIB_DECLARE_OPERATION(
	floor_divide,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("dividend", "divisor"),
	binary_elementwise_shape_rule,
	binary_homogeneous_rule<
		domain_union<real_arithmetic_type_domain, character_type_domain>
	>
);

} // namespace ops
} // namespace rexlib
