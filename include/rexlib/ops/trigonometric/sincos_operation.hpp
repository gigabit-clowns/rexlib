// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

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
	unary_pair_elementwise_shape_rule,
	unary_homogeneous_pair_rule<>
);

} // namespace ops
} // namespace rexlib
