// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/basic_operation.hpp>
#include <rex/ops/ops_component.hpp>
#include <rex/ops/policies/matrix_multiply_shape_policy.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

namespace rex
{
namespace ops
{

/**
 * @brief Multiply two arrays as stacks of matrices.
 *
 * The last two axes of each operand are the matrix and the rest are a
 * stack of them, broadcast between the operands. A one dimensional
 * operand is promoted for the duration, the first as a row and the
 * second as a column, and the axis added to do so is dropped from the
 * result.
 *
 * Integers are admitted: multiplying adjacency or count matrices is
 * meaningful and needs no division.
 */
REX_DECLARE_OPERATION(
	matmul,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("left", "right"),
	matrix_multiply_shape_policy,
	binary_homogeneous_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace rex
