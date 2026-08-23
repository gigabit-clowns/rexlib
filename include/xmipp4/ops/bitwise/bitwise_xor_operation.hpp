// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/basic_operation.hpp>
#include <xmipp4/ops/ops_component.hpp>
#include <xmipp4/ops/policies/elementwise_operation_shape_policy.hpp>
#include <xmipp4/ops/rules/operand_type_rules.hpp>

namespace rex
{
namespace ops
{

/**
 * @brief Compute the bitwise exclusive disjunction of two arrays.
 *
 * Every type with a defined bit pattern is admitted, which
 * excludes only the floating point and complex ones.
 */
REX_DECLARE_OPERATION(
	bitwise_xor,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("left", "right"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<
		domain_difference<any_type_domain, inexact_type_domain>
	>
);

} // namespace ops
} // namespace rex
