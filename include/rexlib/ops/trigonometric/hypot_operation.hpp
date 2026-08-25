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
 * @brief Compute the hypotenuse of two arrays.
 *
 * Computed without the intermediate overflow that squaring the
 * operands would cause.
 */
REXLIB_DECLARE_OPERATION(
	hypot,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("left", "right"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<floating_point_type_domain>
);

} // namespace ops
} // namespace rexlib
