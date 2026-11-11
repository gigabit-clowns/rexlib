// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>
#include <rexlib/core/operation/shape_rule/core_dimensions.hpp>
#include <rexlib/core/operation/shape_rule/extent_pivot.hpp>
#include <rexlib/core/operation/shape_rule/extent_slot.hpp>
#include <rexlib/core/operation/shape_rule/operand_shape_rule.hpp>
#include <rexlib/core/operation/shape_rule/positive_extent_domain.hpp>
#include <rexlib/core/operation/shape_rule/trailing_core.hpp>
#include <rexlib/core/operation/type_rule/operand_type_rule.hpp>
#include <rexlib/ops/rules/axis_half_spectrum_of.hpp>
#include <rexlib/ops/rules/extent_of.hpp>

namespace rexlib
{
namespace em
{

/**
 * @defgroup ctf_rules Rules of the CTF generation
 *
 * The operands of an evaluation of the contrast transfer function over the
 * half spectrum of a signal. The extent variables are, in order: the extent
 * n of the signal, which the operation names, and the number of CTF
 * parameters of a signal. The half spectrum of a 1D signal has the core axis
 * (n / 2 + 1), and that of a square image the core axes (n, n / 2 + 1).
 *
 * The element types are float32 or float64, shared by both operands.
 *
 * @{
 */

/**
 * @brief Core axis of the half spectrum of a 1D signal.
 */
using ctf_1d_spectrum_core = core_dimensions<ops::axis_half_spectrum_of<0>>;

/**
 * @brief Core axes of the half spectrum of a square image.
 */
using ctf_2d_spectrum_core = core_dimensions<
	axis_same_as<0>,
	ops::axis_half_spectrum_of<0>
>;

/**
 * @brief Core axis of the CTF parameters of a signal.
 */
using ctf_parameter_core = core_dimensions<axis_same_as<1>>;

/**
 * @brief Element types of the CTF generation.
 */
using ctf_type_domain = domain_union<
	single_type_domain<numerical_type::float32>,
	single_type_domain<numerical_type::float64>
>;

/**
 * @brief Shape rule of ctf_1d_operation.
 */
using ctf_1d_shape_rule = operand_shape_rule<
	type_list<
		extent_from_operation<ops::extent_of, positive_extent_domain>,
		extent_fixed<2>
	>,
	type_list<ctf_1d_spectrum_core>,
	type_list<ctf_parameter_core>,
	trailing_core
>;

/**
 * @brief Shape rule of ctf_2d_operation.
 */
using ctf_2d_shape_rule = operand_shape_rule<
	type_list<
		extent_from_operation<ops::extent_of, positive_extent_domain>,
		extent_fixed<4>
	>,
	type_list<ctf_2d_spectrum_core>,
	type_list<ctf_parameter_core>,
	trailing_core
>;

/**
 * @brief Type rule of ctf_1d_operation and ctf_2d_operation.
 */
using ctf_type_rule = operand_type_rule<
	type_list<pivot_from_input<0, ctf_type_domain>>,
	type_list<slot_same_as<0>>,
	type_list<slot_same_as<0>>
>;

/** @} */

} // namespace em
} // namespace rexlib
