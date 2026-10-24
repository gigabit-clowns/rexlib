// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/parameters/reduction_axes.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @brief Contract two arrays along one axis, broadcasting the rest.
 *
 * The products are summed along one axis, which holds as many elements in
 * both operands and which the result loses. Every other axis is a batch
 * dimension, broadcast between the operands, so a stack of vectors
 * contracts with another stack, or with a single vector, without any of
 * them being written out.
 *
 * This is where it differs from dot, which keeps the leading axes of both
 * operands rather than broadcasting them, and from matmul, which treats
 * the last two axes as a matrix rather than the last one as a vector.
 *
 * @see dot
 * @see matmul
 */
REXLIB_DECLARE_PARAMETRIC_OPERATION(
	vecdot,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("left", "right"),
	vector_dot_shape_rule,
	reduction_axes,
	binary_homogeneous_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace rexlib
