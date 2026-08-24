// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/basic_operation.hpp>
#include <rex/ops/ops_component.hpp>
#include <rex/ops/policies/vector_matrix_shape_policy.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

namespace rex
{
namespace ops
{

/**
 * @brief Multiply a stack of conjugated vectors by a stack of matrices.
 *
 * The vector's last axis is contracted with the matrix's second to last
 * axis, and everything before is a stack broadcast between the operands.
 * The mirror image of matvec: neither operand is promoted, so the vector
 * must already have rank one or more and the matrix rank two or more.
 *
 * The vector is conjugated before the contraction, the same way vecdot
 * conjugates its own first operand: this is what NumPy's vecmat does,
 * treating the vector as the row of an inner-product-like contraction
 * rather than as a plain linear map. matvec's vector is not conjugated,
 * since it plays that latter role instead.
 *
 * @see matvec
 * @see matmul
 */
REXLIB_DECLARE_OPERATION(
	vecmat,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("left", "right"),
	vector_matrix_shape_policy,
	binary_homogeneous_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace rex
