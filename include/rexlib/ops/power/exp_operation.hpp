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
 * @brief Raise e to the power of each element of the input array.
 */
REXLIB_DECLARE_OPERATION(
	exp,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	unary_elementwise_shape_rule,
	unary_homogeneous_rule<inexact_type_domain>
);

} // namespace ops
} // namespace rexlib
