// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/ops/rules/half_spectrum_extent_transform.hpp>

#include <rexlib/core/operation/shape_rule/extent_slot.hpp>

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
using axis_half_spectrum_of = extent_slot<E, half_spectrum_extent_transform>;

} // namespace ops
} // namespace rexlib
