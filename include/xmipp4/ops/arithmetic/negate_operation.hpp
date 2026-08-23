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
 * @brief Negate each element of the input array into the output array.
 *
 * Only types that can represent a negative value are admitted, which rules
 * out booleans, characters and unsigned integers.
 */
REX_DECLARE_OPERATION(
	negate,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_homogeneous_rule<signed_arithmetic_type_domain>
);

} // namespace ops
} // namespace rex
