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
 * @brief Compute the argument of each element of the input array.
 *
 * The result is the real counterpart of the operand type, an angle
 * having no imaginary part. Integral operands are not admitted: the
 * argument of an integer is a degenerate choice between zero and pi.
 */
REXLIB_DECLARE_OPERATION(
	angle,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_real_of_rule<inexact_type_domain>
);

} // namespace ops
} // namespace rexlib
