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
 * @brief Compute the arc tangent of two arrays, by quadrant.
 *
 * The operands are the ordinate first and the abscissa second,
 * matching std::atan2 rather than reading left to right.
 */
REXLIB_DECLARE_OPERATION(
	atan2,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("y", "x"),
	binary_elementwise_shape_rule,
	binary_homogeneous_rule<floating_point_type_domain>
);

} // namespace ops
} // namespace rexlib
