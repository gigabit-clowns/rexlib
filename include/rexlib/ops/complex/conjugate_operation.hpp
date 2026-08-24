// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/basic_operation.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/policies/elementwise_operation_shape_policy.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @brief Conjugate each element of the input array.
 *
 * Real operands are admitted and left unchanged, so that code
 * generic over the element type does not need a special case.
 */
REXLIB_DECLARE_OPERATION(
	conjugate,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_homogeneous_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace rexlib
