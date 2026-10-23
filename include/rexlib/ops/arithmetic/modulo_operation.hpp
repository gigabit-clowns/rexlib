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
 * @brief Compute the division remainder of two input arrays.
 *
 * The result takes the sign of the divisor, as in Python, rather than the
 * sign of the dividend. Complex and boolean operands are not admitted,
 * having no ordering to take a remainder against.
 */
REXLIB_DECLARE_OPERATION(
	modulo,
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
