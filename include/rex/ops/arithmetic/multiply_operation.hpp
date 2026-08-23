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
 * @brief Multiply the elements of two input arrays into an output array.
 *
 * Boolean operands are multiplied as a conjunction.
 */
REX_DECLARE_OPERATION(
	multiply,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("left", "right"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<>
);

} // namespace ops
} // namespace rex
