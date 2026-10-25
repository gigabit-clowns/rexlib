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
 * @brief Subtract the elements of two input arrays into an output array.
 *
 * Boolean operands are subtracted as an exclusive disjunction.
 */
REXLIB_DECLARE_OPERATION(
	subtract,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("left", "right"),
	binary_elementwise_shape_rule,
	binary_homogeneous_rule<>
);

} // namespace ops
} // namespace rexlib
