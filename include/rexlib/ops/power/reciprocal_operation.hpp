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
 * @brief Divide one by each element of the input array.
 *
 * Integral operands are not admitted: the reciprocal of every integer of
 * magnitude above one is zero, which is a trap rather than a result.
 */
REXLIB_DECLARE_OPERATION(
	reciprocal,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_homogeneous_rule<inexact_type_domain>
);

} // namespace ops
} // namespace rexlib
