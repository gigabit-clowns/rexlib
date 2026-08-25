// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/parametric_operation.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/policies/cross_product_shape_policy.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @brief Take the cross product of two arrays along one axis.
 *
 * The operands are broadcast together and the product is taken along one
 * axis, which survives with the three components it had. Every other axis
 * is a batch dimension.
 *
 * @see vecdot
 */
REXLIB_DECLARE_PARAMETRIC_OPERATION(
	cross,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("left", "right"),
	cross_product_shape_policy,
	binary_homogeneous_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace rexlib
