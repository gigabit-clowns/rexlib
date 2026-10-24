// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/parameters/reduction_axes.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @brief Take the smallest element along the reduced axes.
 *
 * Named apart from the elementwise minimum because operations share one
 * namespace whatever directory they live in, and because min is a macro
 * in windows.h.
 */
REXLIB_DECLARE_PARAMETRIC_OPERATION(
	amin,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	extremum_reduction_shape_rule,
	reduction_axes,
	unary_homogeneous_rule<
		domain_difference<any_type_domain, complex_type_domain>
	>
);

} // namespace ops
} // namespace rexlib
