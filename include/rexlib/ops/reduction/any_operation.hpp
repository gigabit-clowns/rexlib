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
 * @brief Test whether any element along the reduced axes is true.
 */
REXLIB_DECLARE_PARAMETRIC_OPERATION(
	any,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	reduction_shape_rule,
	reduction_axes,
	unary_predicate_rule<>
);

} // namespace ops
} // namespace rexlib
