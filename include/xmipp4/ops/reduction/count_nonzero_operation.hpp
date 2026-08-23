// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/parametric_operation.hpp>
#include <xmipp4/ops/ops_component.hpp>
#include <xmipp4/ops/policies/reduction_operation_shape_policy.hpp>
#include <xmipp4/ops/rules/operand_type_rules.hpp>

namespace xmipp4
{
namespace ops
{

/**
 * @brief Count the non zero elements along the reduced axes.
 */
REX_DECLARE_PARAMETRIC_OPERATION(
	count_nonzero,
	ops_component,
	REX_OPERANDS("count"),
	REX_OPERANDS("value"),
	reduction_operation_shape_policy,
	unary_fixed_output_rule<
		numerical_type::int64,
		any_type_domain
	>
);

} // namespace ops
} // namespace xmipp4
