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
 * @brief Test whether each element is neither infinite nor not a number.
 *
 * Only inexact types can hold either, so an integer array would answer
 * true everywhere.
 */
REXLIB_DECLARE_OPERATION(
	is_finite,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_predicate_rule<inexact_type_domain>
);

} // namespace ops
} // namespace rex
