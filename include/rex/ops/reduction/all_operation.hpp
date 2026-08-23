// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/parametric_operation.hpp>
#include <rex/ops/ops_component.hpp>
#include <rex/ops/policies/reduction_operation_shape_policy.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

namespace rex
{
namespace ops
{

/**
 * @brief Test whether every element along the reduced axes is true.
 *
 * An element counts as true when it is non zero, so any type is
 * admitted and the result is boolean whatever it was.
 */
REX_DECLARE_PARAMETRIC_OPERATION(
	all,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value"),
	reduction_operation_shape_policy,
	unary_predicate_rule<>
);

} // namespace ops
} // namespace rex
