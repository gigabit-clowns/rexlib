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
 * @brief Test whether any element along the reduced axes is true.
 */
REX_DECLARE_PARAMETRIC_OPERATION(
	any,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value"),
	reduction_operation_shape_policy,
	unary_predicate_rule<>
);

} // namespace ops
} // namespace xmipp4
