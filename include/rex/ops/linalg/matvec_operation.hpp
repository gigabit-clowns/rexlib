// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/basic_operation.hpp>
#include <rex/ops/ops_component.hpp>
#include <rex/ops/policies/matrix_vector_shape_policy.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

namespace rex
{
namespace ops
{

/**
 * @brief Multiply a stack of matrices by a stack of vectors.
 *
 * The matrix's last axis is contracted with the vector's last axis, and
 * everything before is a stack broadcast between the operands, as with
 * matmul. Unlike matmul, neither operand is promoted: the matrix must
 * already have rank two or more and the vector rank one or more.
 *
 * @see vecmat
 * @see matmul
 */
REXLIB_DECLARE_OPERATION(
	matvec,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("left", "right"),
	matrix_vector_shape_policy,
	binary_homogeneous_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace rex
