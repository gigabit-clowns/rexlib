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
	binary_pair_elementwise_shape_rule,
	binary_homogeneous_pair_rule<
		domain_union<real_arithmetic_type_domain, character_type_domain>
	>
);

} // namespace ops
} // namespace rexlib
