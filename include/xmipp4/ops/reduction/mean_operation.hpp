// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/parametric_operation.hpp>
#include <xmipp4/ops/ops_component.hpp>
#include <xmipp4/ops/policies/reduction_operation_shape_policy.hpp>
#include <xmipp4/ops/rules/operand_type_rules.hpp>

namespace rex
{
namespace ops
{

/**
 * @brief Average the elements along the reduced axes.
 *
 * The result is the inexact counterpart of the operand type, an average
 * rarely being representable in an exact one. An integer array therefore
 * averages to float64, while a float32 one stays float32.
 */
REX_DECLARE_PARAMETRIC_OPERATION(
	mean,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value"),
	reduction_operation_shape_policy,
	unary_inexact_of_rule<>
);

} // namespace ops
} // namespace rex
