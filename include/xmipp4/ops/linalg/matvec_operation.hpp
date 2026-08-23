// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/basic_operation.hpp>
#include <xmipp4/ops/ops_component.hpp>
#include <xmipp4/ops/policies/matrix_vector_shape_policy.hpp>
#include <xmipp4/ops/rules/operand_type_rules.hpp>

namespace xmipp4
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
REX_DECLARE_OPERATION(
	matvec,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("left", "right"),
	matrix_vector_shape_policy,
	binary_homogeneous_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace xmipp4
