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
 * @brief Take the magnitude of one array and the sign of another.
 */
REXLIB_DECLARE_OPERATION(
	copysign,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("magnitude", "sign"),
	binary_elementwise_shape_rule,
	binary_homogeneous_rule<floating_point_type_domain>
);

} // namespace ops
} // namespace rexlib
