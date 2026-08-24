// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/parametric_operation.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/policies/axiswise_operation_shape_policy.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @brief Move the zero frequency of a spectrum to its centre.
 *
 * Rearranges elements without changing the shape or the type, so any
 * operand is admitted.
 */
REXLIB_DECLARE_PARAMETRIC_OPERATION(
	fftshift,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	axiswise_operation_shape_policy,
	unary_homogeneous_rule<>
);

} // namespace ops
} // namespace rexlib
