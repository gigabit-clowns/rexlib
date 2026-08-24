// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/parametric_operation.hpp>
#include <rex/ops/ops_component.hpp>
#include <rex/ops/policies/reduction_operation_shape_policy.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

namespace rex
{
namespace ops
{

/**
 * @brief Locate the largest element along the reduced axes.
 */
REXLIB_DECLARE_PARAMETRIC_OPERATION(
	argmax,
	ops_component,
	REXLIB_OPERANDS("index"),
	REXLIB_OPERANDS("value"),
	reduction_operation_shape_policy,
	unary_fixed_output_rule<
		numerical_type::int64,
		domain_difference<any_type_domain, complex_type_domain>
	>
);

} // namespace ops
} // namespace rex
