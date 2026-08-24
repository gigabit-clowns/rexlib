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
 * @brief Test whether the elements of one array are less than or equal to
 * those of another.
 *
 * Complex operands are not admitted: the complex plane has no
 * ordering, and comparing the parts lexicographically would be a
 * convention rather than a meaning.
 */
REXLIB_DECLARE_OPERATION(
	less_equal,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("left", "right"),
	elementwise_operation_shape_policy,
	binary_predicate_rule<
		domain_difference<any_type_domain, complex_type_domain>
	>
);

} // namespace ops
} // namespace rexlib
