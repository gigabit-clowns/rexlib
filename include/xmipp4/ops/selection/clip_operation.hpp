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
 * @brief Confine the elements of an array to an interval.
 *
 * The bounds are operands rather than parameters, so they broadcast
 * against the value like any other operand and may vary across it.
 *
 * Complex operands are not admitted, there being no ordering to
 * clamp against.
 */
REX_DECLARE_OPERATION(
	clip,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value", "lower", "upper"),
	elementwise_operation_shape_policy,
	ternary_homogeneous_rule<
		domain_difference<any_type_domain, complex_type_domain>
	>
);

} // namespace ops
} // namespace xmipp4
