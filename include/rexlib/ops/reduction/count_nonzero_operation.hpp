// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/parametric_operation.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/policies/reduction_operation_shape_policy.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @brief Count the non zero elements along the reduced axes.
 */
REXLIB_DECLARE_PARAMETRIC_OPERATION(
	count_nonzero,
	ops_component,
	REXLIB_OPERANDS("count"),
	REXLIB_OPERANDS("value"),
	reduction_operation_shape_policy,
	unary_fixed_output_rule<
		numerical_type::int64,
		any_type_domain
	>
);

} // namespace ops
} // namespace rexlib
