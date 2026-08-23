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
 * @brief Locate the smallest element along the reduced axes.
 *
 * The result holds indices rather than values, so it is int64 whatever
 * the operand type was.
 */
REX_DECLARE_PARAMETRIC_OPERATION(
	argmin,
	ops_component,
	REX_OPERANDS("index"),
	REX_OPERANDS("value"),
	reduction_operation_shape_policy,
	unary_fixed_output_rule<
		numerical_type::int64,
		domain_difference<any_type_domain, complex_type_domain>
	>
);

} // namespace ops
} // namespace xmipp4
