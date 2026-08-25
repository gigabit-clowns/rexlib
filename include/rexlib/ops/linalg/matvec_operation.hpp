// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/basic_operation.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/policies/matrix_vector_shape_policy.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

namespace rexlib
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
} // namespace rexlib
