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
 * @brief Copy elements from an input array to an output array.
 *
 * The destination adopts the source element type unless the caller
 * pre-allocates one of a different type, which is what makes a converting
 * copy possible without a separate operation.
 */
REXLIB_DECLARE_OPERATION(
	copy,
	ops_component,
	REXLIB_OPERANDS("destination"),
	REXLIB_OPERANDS("source"),
	unary_elementwise_shape_rule,
	converting_rule<>
);

} // namespace ops
} // namespace rexlib
