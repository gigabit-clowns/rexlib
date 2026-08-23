// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/basic_operation.hpp>
#include <xmipp4/ops/ops_component.hpp>
#include <xmipp4/ops/policies/elementwise_operation_shape_policy.hpp>
#include <xmipp4/ops/rules/operand_type_rules.hpp>

namespace rex
{
namespace ops
{

/**
 * @brief Copy elements from an input array to an output array.
 *
 * The destination adopts the source element type unless the caller
 * pre-allocates one of a different type, which is what makes a converting
 * copy possible without a separate operation.
 */
REX_DECLARE_OPERATION(
	copy,
	ops_component,
	REX_OPERANDS("destination"),
	REX_OPERANDS("source"),
	elementwise_operation_shape_policy,
	converting_rule<>
);

} // namespace ops
} // namespace rex
