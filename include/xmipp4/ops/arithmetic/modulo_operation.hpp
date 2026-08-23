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
 * @brief Compute the division remainder of two input arrays.
 *
 * The result takes the sign of the divisor, as in Python, rather than the
 * sign of the dividend. Complex and boolean operands are not admitted,
 * having no ordering to take a remainder against.
 */
REX_DECLARE_OPERATION(
	modulo,
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
