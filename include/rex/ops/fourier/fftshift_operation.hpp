// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/parametric_operation.hpp>
#include <rex/ops/ops_component.hpp>
#include <rex/ops/policies/axiswise_operation_shape_policy.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

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
