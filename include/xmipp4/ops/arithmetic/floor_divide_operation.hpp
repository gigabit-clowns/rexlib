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
 * @brief Divide two input arrays, rounding the quotient downwards.
 *
 * The quotient is rounded towards negative infinity rather than towards
 * zero, so that it pairs with the remainder computed by modulo. Complex
 * operands are not admitted, having no ordering to round against.
 */
REX_DECLARE_OPERATION(
	floor_divide,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("dividend", "divisor"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<
		domain_union<real_arithmetic_type_domain, character_type_domain>
	>
);

} // namespace ops
} // namespace xmipp4
