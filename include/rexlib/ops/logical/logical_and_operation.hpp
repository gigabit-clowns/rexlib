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
 * @brief Compute the conjunction of two arrays.
 *
 * Operands of any type are admitted, an element being true when
 * it is non zero, and the result is boolean whatever they were.
 */
REXLIB_DECLARE_OPERATION(
	logical_and,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("left", "right"),
	elementwise_operation_shape_policy,
	binary_predicate_rule<>
);

} // namespace ops
} // namespace rexlib
