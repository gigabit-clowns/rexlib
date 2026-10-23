// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/basic_operation.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/rules/operand_shape_rules.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @brief Take the larger of each pair of elements.
 *
 * Selects one of its operands rather than computing a new value, which is
 * what places it here rather than among the comparisons: it is the lower
 * bound of a @ref clip_operation taken on its own.
 *
 * Complex operands are not admitted, there being no ordering to
 * select by.
 */
REXLIB_DECLARE_OPERATION(
	maximum,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("left", "right"),
	binary_elementwise_shape_rule,
	binary_homogeneous_rule<
		domain_difference<any_type_domain, complex_type_domain>
	>
);

} // namespace ops
} // namespace rexlib
