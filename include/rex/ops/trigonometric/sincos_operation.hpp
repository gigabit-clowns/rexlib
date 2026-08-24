// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/basic_operation.hpp>
#include <rex/ops/ops_component.hpp>
#include <rex/ops/policies/elementwise_operation_shape_policy.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

namespace rexlib
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
REXLIB_DECLARE_OPERATION(
	sincos,
	ops_component,
	REXLIB_OPERANDS("sine", "cosine"),
	REXLIB_OPERANDS("angle"),
	elementwise_operation_shape_policy,
	unary_homogeneous_pair_rule<>
);

} // namespace ops
} // namespace rexlib
