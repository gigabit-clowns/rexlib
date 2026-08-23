// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/basic_operation.hpp>
#include <rex/ops/ops_component.hpp>
#include <rex/ops/policies/elementwise_operation_shape_policy.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

namespace rex
{
namespace ops
{

/**
 * @brief Divide the elements of two input arrays into an output array.
 *
 * Every type but boolean is admitted, division not being defined on it.
 */
REX_DECLARE_OPERATION(
	divide,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("dividend", "divisor"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<
		domain_difference<any_type_domain, boolean_type_domain>
	>
);

} // namespace ops
} // namespace rex
