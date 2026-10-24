// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/parameters/reduction_axes.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

namespace rexlib
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
REXLIB_DECLARE_PARAMETRIC_OPERATION(
	mean,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	reduction_shape_rule,
	reduction_axes,
	unary_inexact_of_rule<>
);

} // namespace ops
} // namespace rexlib
