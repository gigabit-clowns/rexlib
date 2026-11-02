// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Window placing a sample on the nearest voxel of an axis.
 *
 * A sample halfway between two voxels goes to the one farther from 0, so
 * that a sample at -c mirrors the sample at c.
 */
class box_window
{
public:
	/**
	 * @brief Number of voxels a sample is placed on.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t tap_count = 1;

	/**
	 * @brief Get the number of voxels a sample is placed on.
	 *
	 * @return std::size_t 1.
	 */
	std::size_t get_tap_count() const noexcept;

	/**
	 * @brief Place a sample on the voxels of an axis.
	 *
	 * @tparam T The type of the weights.
	 * @param coordinate The position of the sample along the axis, in
	 * voxels.
	 * @param weights Receives @ref tap_count weights, one per voxel from the
	 * first.
	 * @return std::ptrdiff_t The index of the first voxel.
	 */
	template <typename T>
	std::ptrdiff_t place(double coordinate, T *weights) const noexcept;
};

} // namespace cpu
} // namespace rexlib

#include "box_window.inl"
