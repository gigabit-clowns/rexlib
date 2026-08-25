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
 * @brief Take the largest element along the reduced axes.
 *
 * Named apart from the elementwise maximum, as amin is.
 */
REXLIB_DECLARE_PARAMETRIC_OPERATION(
	amax,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	reduction_operation_shape_policy,
	unary_homogeneous_rule<
		domain_difference<any_type_domain, complex_type_domain>
	>
);

} // namespace ops
} // namespace rexlib
