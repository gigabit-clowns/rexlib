// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/basic_operation.hpp>
#include <rex/ops/ops_component.hpp>
#include <rex/ops/policies/elementwise_operation_shape_policy.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

namespace rex
{
namespace ops
{

/**
 * @brief Compute the negation of an array.
 *
 * An element is true when it is non zero, and the result is boolean
 * whatever the operand was.
 */
REX_DECLARE_OPERATION(
	logical_not,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_predicate_rule<>
);

} // namespace ops
} // namespace rex
