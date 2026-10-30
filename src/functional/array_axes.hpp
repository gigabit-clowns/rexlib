// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/ops/parameters/axis_list.hpp>

#include <cstddef>

namespace rexlib
{

/**
 * @brief Get the number of axes of an array.
 *
 * @param x The array.
 * @return std::size_t The rank.
 */
std::size_t get_rank(const const_array_ref &x);

/**
 * @brief Resolve the axes a caller names against the rank of an array.
 *
 * Negative axes count from the end.
 *
 * @param x The array.
 * @param axes The axes named.
 * @return ops::axis_list The axes, each in [0, rank).
 * @throws std::out_of_range When an axis lies outside [-rank, rank).
 */
ops::axis_list resolve_axes(
	const const_array_ref &x,
	span<const std::ptrdiff_t> axes
);

/**
 * @brief Name the last axes of an array, in order.
 *
 * @param x The array.
 * @param count The number of axes named.
 * @return ops::axis_list The axes rank - count to rank - 1.
 * @throws std::invalid_argument When the array has fewer than @p count
 * axes.
 */
ops::axis_list last_axes(const const_array_ref &x, std::size_t count);

/**
 * @brief Name every axis of an array, in order.
 *
 * @param x The array.
 * @return ops::axis_list The axes 0 to rank - 1.
 */
ops::axis_list every_axis(const const_array_ref &x);

} // namespace rexlib
