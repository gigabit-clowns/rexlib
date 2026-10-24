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
 * @brief Test whether the sign bit of each element is set.
 *
 * Its reason to exist is telling negative zero from positive zero,
 * which only a floating point type has. For an integer array,
 * comparing against zero says the same thing more plainly.
 */
REXLIB_DECLARE_OPERATION(
	sign_bit,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	unary_elementwise_shape_rule,
	unary_predicate_rule<floating_point_type_domain>
);

} // namespace ops
} // namespace rexlib
