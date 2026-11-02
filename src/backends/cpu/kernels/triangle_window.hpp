// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Window placing a sample on the two voxels around it, with linear
 * weights.
 *
 * The voxel below the sample receives 1 - f and the one above f, where f is
 * the fractional part of the position.
 */
class triangle_window
{
public:
	/**
	 * @brief Number of voxels a sample is placed on.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t tap_count = 2;

	/**
	 * @brief Get the number of voxels a sample is placed on.
	 *
	 * @return std::size_t 2.
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
	 * @return std::ptrdiff_t The index of the first voxel, the one below the
	 * sample.
	 */
	template <typename T>
	std::ptrdiff_t place(double coordinate, T *weights) const noexcept;
};

} // namespace cpu
} // namespace rexlib

#include "triangle_window.inl"
