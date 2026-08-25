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
 * @brief Test whether any element along the reduced axes is true.
 */
REXLIB_DECLARE_PARAMETRIC_OPERATION(
	any,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	reduction_operation_shape_policy,
	unary_predicate_rule<>
);

} // namespace ops
} // namespace rexlib
