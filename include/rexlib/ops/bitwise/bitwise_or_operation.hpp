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
 * @brief Compute the bitwise disjunction of two arrays.
 *
 * Every type with a defined bit pattern is admitted, which
 * excludes only the floating point and complex ones.
 */
REXLIB_DECLARE_OPERATION(
	bitwise_or,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("left", "right"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<
		domain_difference<any_type_domain, inexact_type_domain>
	>
);

} // namespace ops
} // namespace rexlib
