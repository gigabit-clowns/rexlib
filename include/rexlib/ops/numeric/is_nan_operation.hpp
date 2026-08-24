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
 * @brief Test whether each element is not a number.
 *
 * Only inexact types can hold one, so asking an integer array is always a
 * mistake rather than a constant answer.
 */
REXLIB_DECLARE_OPERATION(
	is_nan,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_predicate_rule<inexact_type_domain>
);

} // namespace ops
} // namespace rexlib
