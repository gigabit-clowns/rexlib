// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/ops/rules/half_spectrum_extent_transform.hpp>

#include <rexlib/core/operation/shape_rule/any_extent_domain.hpp>
#include <rexlib/core/operation/shape_rule/operand_axis_slot.hpp>

#include <cstddef>

namespace rexlib
{
namespace ops
{

/**
 * @brief Axis holding the half spectrum of an extent variable.
 *
 * @tparam E Index of the extent variable.
 */
template <std::size_t E>
using axis_half_spectrum_of = operand_axis_slot<
	axis_slot_kind::extent,
	E,
	half_spectrum_extent_transform,
	any_extent_domain
>;

} // namespace ops
} // namespace rexlib
