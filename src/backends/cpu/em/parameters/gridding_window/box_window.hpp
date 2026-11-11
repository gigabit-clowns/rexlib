// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>
#include <cstdint>

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
	 * @brief Place a run of samples on the voxels of an axis.
	 *
	 * @tparam T The type of the positions and the weights.
	 * @param coordinates The position of each sample along the axis, in
	 * voxels. Within the range of std::int32_t.
	 * @param count The number of samples.
	 * @param first Receives the index of the first voxel of each sample.
	 * @param weights Receives @ref tap_count weights per sample, the weights
	 * of tap t from weights + t * weight_stride.
	 * @param weight_stride The distance between the weights of two taps.
	 */
	template <typename T>
	void place(
		const T *coordinates,
		std::int32_t count,
		std::int32_t *first,
		T *weights,
		std::size_t weight_stride
	) const noexcept;
};

} // namespace cpu
} // namespace rexlib

#include "box_window.inl"
