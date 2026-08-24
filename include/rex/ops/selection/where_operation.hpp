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
 * @brief Select between two arrays according to a condition.
 *
 * The element type follows the arrays being selected from and not
 * the condition, which is always boolean.
 */
REXLIB_DECLARE_OPERATION(
	where,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("condition", "on_true", "on_false"),
	elementwise_operation_shape_policy,
	selection_rule<>
);

} // namespace ops
} // namespace rexlib
