// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/parametric_operation.hpp>
#include <rex/ops/ops_component.hpp>
#include <rex/ops/policies/reduction_operation_shape_policy.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

namespace rex
{
namespace ops
{

/**
 * @brief Contract two arrays along one axis, broadcasting the rest.
 *
 * The operands are broadcast together and the products are summed along
 * one axis, which the result loses. Every other axis is a batch dimension,
 * so a stack of vectors contracts with another stack, or with a single
 * vector, without any of them being written out.
 *
 * This is where it differs from dot, which keeps the leading axes of both
 * operands rather than broadcasting them, and from matmul, which treats
 * the last two axes as a matrix rather than the last one as a vector.
 *
 * Being a broadcast followed by a sum along an axis, it is exactly what the
 * reduction shape policy already describes.
 *
 * @see dot
 * @see matmul
 */
REX_DECLARE_PARAMETRIC_OPERATION(
	vecdot,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("left", "right"),
	reduction_operation_shape_policy,
	binary_homogeneous_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace rex
