// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/basic_operation.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/policies/elementwise_operation_shape_policy.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @brief Compute the negation of an array.
 *
 * An element is true when it is non zero, and the result is boolean
 * whatever the operand was.
 */
REXLIB_DECLARE_OPERATION(
	logical_not,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_predicate_rule<>
);

} // namespace ops
} // namespace rexlib
