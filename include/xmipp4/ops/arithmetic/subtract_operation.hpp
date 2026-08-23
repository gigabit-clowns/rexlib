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
 * @brief Subtract the elements of two input arrays into an output array.
 *
 * Boolean operands are subtracted as an exclusive disjunction.
 */
REX_DECLARE_OPERATION(
	subtract,
	ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("left", "right"),
	elementwise_operation_shape_policy,
	binary_homogeneous_rule<>
);

} // namespace ops
} // namespace xmipp4
