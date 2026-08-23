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
 * @brief Test whether each element is infinite.
 *
 * Only inexact types can hold an infinity.
 */
REX_DECLARE_OPERATION(
	is_inf,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_predicate_rule<inexact_type_domain>
);

} // namespace ops
} // namespace rex
