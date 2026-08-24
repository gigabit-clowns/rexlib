// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/parametric_operation.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/policies/reduction_operation_shape_policy.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @brief Test whether every element along the reduced axes is true.
 *
 * An element counts as true when it is non zero, so any type is
 * admitted and the result is boolean whatever it was.
 */
REXLIB_DECLARE_PARAMETRIC_OPERATION(
	all,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	reduction_operation_shape_policy,
	unary_predicate_rule<>
);

} // namespace ops
} // namespace rexlib
