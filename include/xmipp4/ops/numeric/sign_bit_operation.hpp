// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/basic_operation.hpp>
#include <xmipp4/ops/ops_component.hpp>
#include <xmipp4/ops/policies/elementwise_operation_shape_policy.hpp>
#include <xmipp4/ops/rules/operand_type_rules.hpp>

namespace rex
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
REX_DECLARE_OPERATION(
	sign_bit,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_predicate_rule<floating_point_type_domain>
);

} // namespace ops
} // namespace rex
