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
 * @brief Add the elements of the input array along the reduced axes.
 *
 * Every type add accepts is admitted, including boolean, whose sum is
 * a disjunction. Overflow is a question about values rather than about
 * types, and is not one a domain can answer.
 */
REXLIB_DECLARE_PARAMETRIC_OPERATION(
	sum,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	reduction_shape_rule,
	reduction_axes,
	unary_homogeneous_rule<>
);

} // namespace ops
} // namespace rexlib
