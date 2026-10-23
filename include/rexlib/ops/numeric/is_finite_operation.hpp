// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/rules/operand_shape_rules.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @brief Test whether each element is neither infinite nor not a number.
 *
 * Only inexact types can hold either, so an integer array would answer
 * true everywhere.
 */
REXLIB_DECLARE_OPERATION(
	is_finite,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	unary_elementwise_shape_rule,
	unary_predicate_rule<inexact_type_domain>
);

} // namespace ops
} // namespace rexlib
