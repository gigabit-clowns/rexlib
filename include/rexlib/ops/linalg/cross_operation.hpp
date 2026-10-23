// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/parameters/axis_selection.hpp>
#include <rexlib/ops/rules/operand_shape_rules.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @brief Take the cross product of two arrays along one axis.
 *
 * The product is taken along one axis, which holds three components in
 * both operands and survives in the result. Every other axis is a batch
 * dimension, broadcast between the operands.
 *
 * @see vecdot
 */
REXLIB_DECLARE_PARAMETRIC_OPERATION(
	cross,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("left", "right"),
	cross_product_shape_rule,
	axis_selection,
	binary_homogeneous_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace rexlib
