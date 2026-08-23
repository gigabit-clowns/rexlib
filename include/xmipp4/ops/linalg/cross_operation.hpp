// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/parametric_operation.hpp>
#include <xmipp4/ops/ops_component.hpp>
#include <xmipp4/ops/policies/cross_product_shape_policy.hpp>
#include <xmipp4/ops/rules/operand_type_rules.hpp>

namespace xmipp4
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
REX_DECLARE_PARAMETRIC_OPERATION(
	cross,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("left", "right"),
	cross_product_shape_policy,
	binary_homogeneous_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace xmipp4
