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
 * @brief Raise the elements of an array to the power of another.
 *
 * A negative integer exponent has no representable result and is a value
 * error rather than a typing one, so it is left for the backend to
 * report.
 */
REX_DECLARE_OPERATION(
	power,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("base", "exponent"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<arithmetic_type_domain>
);

} // namespace ops
} // namespace xmipp4
