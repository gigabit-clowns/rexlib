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
 * @brief Compute the natural logarithm of one plus each element.
 *
 * Accurate for operands close to zero, where adding one first loses the
 * significant digits.
 */
REXLIB_DECLARE_OPERATION(
	log1p,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_homogeneous_rule<floating_point_type_domain>
);

} // namespace ops
} // namespace rexlib
