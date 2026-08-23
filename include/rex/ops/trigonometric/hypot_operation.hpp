// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/basic_operation.hpp>
#include <rex/ops/ops_component.hpp>
#include <rex/ops/policies/elementwise_operation_shape_policy.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

namespace rex
{
namespace ops
{

/**
 * @brief Compute the hypotenuse of two arrays.
 *
 * Computed without the intermediate overflow that squaring the
 * operands would cause.
 */
REX_DECLARE_OPERATION(
	hypot,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("left", "right"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<floating_point_type_domain>
);

} // namespace ops
} // namespace rex
