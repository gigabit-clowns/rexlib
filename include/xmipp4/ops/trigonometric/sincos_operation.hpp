// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/basic_operation.hpp>
#include <xmipp4/ops/ops_component.hpp>
#include <xmipp4/ops/policies/elementwise_operation_shape_policy.hpp>
#include <xmipp4/ops/rules/operand_type_rules.hpp>

namespace xmipp4
{
namespace ops
{

/**
 * @brief Compute the sine and cosine of each element at once.
 *
 * The two share the argument reduction that dominates their cost,
 * so computing them together is markedly cheaper than computing
 * them apart. Rotations and projections need both.
 */
REX_DECLARE_OPERATION(
	sincos,
	ops_component,
	REX_OPERANDS("sine", "cosine"),
	REX_OPERANDS("angle"),
	elementwise_operation_shape_policy,
	unary_homogeneous_pair_rule<>
);

} // namespace ops
} // namespace xmipp4
