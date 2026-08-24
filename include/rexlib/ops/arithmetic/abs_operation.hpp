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
 * @brief Compute the absolute value of each element of the input array.
 *
 * The output has the real counterpart of the input type, so the magnitude
 * of a complex array is a real one.
 */
REXLIB_DECLARE_OPERATION(
	abs,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_real_of_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace rexlib
