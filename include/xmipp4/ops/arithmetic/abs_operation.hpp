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
 * @brief Compute the absolute value of each element of the input array.
 *
 * The output has the real counterpart of the input type, so the magnitude
 * of a complex array is a real one.
 */
REX_DECLARE_OPERATION(
	abs,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value"),
	elementwise_operation_shape_policy,
	unary_real_of_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace xmipp4
