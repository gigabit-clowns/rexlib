// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "slice_frame.hpp"

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief A closed box of volume frequencies, one interval per axis.
 */
class frequency_bounds
{
public:
	/**
	 * @brief Construct a box.
	 *
	 * @param lower The lowest frequency of each axis.
	 * @param upper The highest frequency of each axis.
	 */
	frequency_bounds(
		const volume_frequency &lower,
		const volume_frequency &upper
	) noexcept;

	/**
	 * @brief Get the lowest frequency of an axis.
	 *
	 * @param axis 0 for x, 1 for y, 2 for z.
	 * @return double The frequency.
	 */
	double get_lower(std::size_t axis) const noexcept;

	/**
	 * @brief Get the highest frequency of an axis.
	 *
	 * @param axis 0 for x, 1 for y, 2 for z.
	 * @return double The frequency.
	 */
	double get_upper(std::size_t axis) const noexcept;

	/**
	 * @brief Get the box widened by a margin on every side.
	 *
	 * @param margin The margin, in voxels.
	 * @return frequency_bounds The widened box.
	 */
	frequency_bounds widened(double margin) const noexcept;

private:
	volume_frequency m_lower;
	volume_frequency m_upper;
};

} // namespace cpu
} // namespace rexlib
