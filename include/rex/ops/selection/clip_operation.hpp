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
 * @brief Confine the elements of an array to an interval.
 *
 * The bounds are operands rather than parameters, so they broadcast
 * against the value like any other operand and may vary across it.
 *
 * Complex operands are not admitted, there being no ordering to
 * clamp against.
 */
REXLIB_DECLARE_OPERATION(
	clip,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value", "lower", "upper"),
	elementwise_operation_shape_policy,
	ternary_homogeneous_rule<
		domain_difference<any_type_domain, complex_type_domain>
	>
);

} // namespace ops
} // namespace rexlib
