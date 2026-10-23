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
 * @brief Convert each element of the input array to degrees.
 */
REXLIB_DECLARE_OPERATION(
	degrees,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("radians"),
	unary_elementwise_shape_rule,
	unary_homogeneous_rule<floating_point_type_domain>
);

} // namespace ops
} // namespace rexlib
