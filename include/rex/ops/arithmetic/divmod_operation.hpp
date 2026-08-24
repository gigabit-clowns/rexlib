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
 * @brief Divide two arrays, producing the quotient and remainder.
 *
 * Computing both in one pass is cheaper than computing them apart,
 * the division being the expensive half of each. The quotient is
 * the one floor_divide would give and the remainder the one modulo
 * would, so the two agree by construction rather than by
 * convention.
 */
REXLIB_DECLARE_OPERATION(
	divmod,
	ops_component,
	REXLIB_OPERANDS("quotient", "remainder"),
	REXLIB_OPERANDS("dividend", "divisor"),
	elementwise_operation_shape_policy,
	binary_homogeneous_pair_rule<
		domain_union<real_arithmetic_type_domain, character_type_domain>
	>
);

} // namespace ops
} // namespace rexlib
