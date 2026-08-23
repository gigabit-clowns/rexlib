// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/basic_operation.hpp>
#include <xmipp4/ops/ops_component.hpp>
#include <xmipp4/ops/policies/elementwise_operation_shape_policy.hpp>
#include <xmipp4/ops/rules/operand_type_rules.hpp>

namespace xmipp4
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
} // namespace xmipp4
