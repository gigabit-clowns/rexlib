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
 * @brief Divide the elements of two input arrays into an output array.
 *
 * Every type but boolean is admitted, division not being defined on it.
 */
REXLIB_DECLARE_OPERATION(
	divide,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("dividend", "divisor"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<
		domain_difference<any_type_domain, boolean_type_domain>
	>
);

} // namespace ops
} // namespace rexlib
