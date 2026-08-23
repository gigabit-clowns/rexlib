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
 * @brief Take the magnitude of one array and the sign of another.
 */
REX_DECLARE_OPERATION(
	copysign,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("magnitude", "sign"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<floating_point_type_domain>
);

} // namespace ops
} // namespace rex
