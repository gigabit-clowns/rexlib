// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/parametric_operation.hpp>
#include <xmipp4/ops/ops_component.hpp>
#include <xmipp4/ops/policies/axiswise_operation_shape_policy.hpp>
#include <xmipp4/ops/rules/operand_type_rules.hpp>

namespace rex
{
namespace ops
{

/**
 * @brief Move the zero frequency of a spectrum to its centre.
 *
 * Rearranges elements without changing the shape or the type, so any
 * operand is admitted.
 */
REX_DECLARE_PARAMETRIC_OPERATION(
	fftshift,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value"),
	axiswise_operation_shape_policy,
	unary_homogeneous_rule<>
);

} // namespace ops
} // namespace rex
