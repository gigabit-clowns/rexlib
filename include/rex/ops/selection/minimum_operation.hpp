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
 * @brief Take the smaller of each pair of elements.
 *
 * Selects one of its operands rather than computing a new value, which is
 * what places it here rather than among the comparisons: it is the upper
 * bound of a @ref clip_operation taken on its own.
 *
 * Complex operands are not admitted, there being no ordering to
 * select by.
 */
REX_DECLARE_OPERATION(
	minimum,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("left", "right"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<
		domain_difference<any_type_domain, complex_type_domain>
	>
);

} // namespace ops
} // namespace rex
