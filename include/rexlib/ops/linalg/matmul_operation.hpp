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
 * @brief Multiply two arrays as stacks of matrices.
 *
 * The last two axes of each operand are the matrix and the rest are a
 * stack of them, broadcast between the operands. Both operands must have
 * at least two axes: a vector is not promoted to a matrix.
 *
 * Integers are admitted: multiplying adjacency or count matrices is
 * meaningful and needs no division.
 */
REXLIB_DECLARE_OPERATION(
	matmul,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("left", "right"),
	matrix_multiply_shape_rule,
	binary_homogeneous_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace rexlib
