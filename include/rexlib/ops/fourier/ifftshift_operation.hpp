// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/parameters/axis_selection.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @brief Move the zero frequency of a spectrum back to its origin.
 *
 * The inverse of fftshift, which it differs from only for axes of odd
 * extent.
 */
REXLIB_DECLARE_PARAMETRIC_OPERATION(
	ifftshift,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	axiswise_shape_rule,
	axis_selection,
	unary_homogeneous_rule<>
);

} // namespace ops
} // namespace rexlib
