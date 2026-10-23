// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/parametric_operation.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/parameters/axis_selection.hpp>
#include <rexlib/ops/rules/operand_shape_rules.hpp>
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
	axiswise_shape_rule,
	axis_selection,
	unary_homogeneous_rule<>
);

} // namespace ops
} // namespace rexlib
