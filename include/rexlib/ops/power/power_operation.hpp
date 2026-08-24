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
 * @brief Raise the elements of an array to the power of another.
 *
 * A negative integer exponent has no representable result and is a value
 * error rather than a typing one, so it is left for the backend to
 * report.
 */
REXLIB_DECLARE_OPERATION(
	power,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("base", "exponent"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace rexlib
