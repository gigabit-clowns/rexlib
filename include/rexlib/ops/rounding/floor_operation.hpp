// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @brief Round each element of the input array downwards.
 *
 * Integral operands are not admitted: rounding one is a copy, and
 * asking for it is more likely a mistake than an intention.
 */
REXLIB_DECLARE_OPERATION(
	floor,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	unary_elementwise_shape_rule,
	unary_homogeneous_rule<floating_point_type_domain>
);

} // namespace ops
} // namespace rexlib
