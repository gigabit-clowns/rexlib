// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/span.hpp>

#include <cstddef>
#include <type_traits>
#include <vector>

namespace rexlib
{

/**
 * @brief Select the values of some axes.
 *
 * @tparam T The type of the values, const or not.
 * @param values One value per axis, such as the extents or the strides of a
 * layout.
 * @param axes The axes to select, each below the size of values. An axis
 * may appear more than once.
 * @return std::vector The value of each axis, in the order of axes.
 */
template <typename T>
std::vector<typename std::remove_const<T>::type> select_axes(
	span<T> values,
	span<const std::size_t> axes
);

} // namespace rexlib

#include "axis_values.inl"
