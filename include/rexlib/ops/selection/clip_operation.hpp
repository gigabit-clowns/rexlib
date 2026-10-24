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
	ternary_elementwise_shape_rule,
	ternary_homogeneous_rule<
		domain_difference<any_type_domain, complex_type_domain>
	>
);

} // namespace ops
} // namespace rexlib
