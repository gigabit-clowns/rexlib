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
 * @brief Take the smallest element along the reduced axes.
 *
 * Named apart from the elementwise minimum because operations share one
 * namespace whatever directory they live in, and because min is a macro
 * in windows.h.
 */
REX_DECLARE_PARAMETRIC_OPERATION(
	amin,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value"),
	reduction_operation_shape_policy,
	unary_homogeneous_rule<
		domain_difference<any_type_domain, complex_type_domain>
	>
);

} // namespace ops
} // namespace xmipp4
