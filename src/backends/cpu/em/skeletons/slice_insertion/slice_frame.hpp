// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <array>

namespace rexlib
{
namespace cpu
{

/**
 * @brief A frequency of a volume, as (x, y, z) in voxels of its spectrum.
 */
using volume_frequency = std::array<double, 3>;

/**
 * @brief Where the frequencies of a slice land in the spectrum of a volume.
 *
 * A slice frequency (k_x, k_y) lands on K = k_x X + k_y Y, where X and Y
 * are the first two rows of the transform of the slice scaled by the ratio
 * of the extents of the volume and the slice.
 */
class slice_frame
{
public:
	/**
	 * @brief Construct the frame of a slice.
	 *
	 * @param x_row The first row of the transform, (A00, A01, A02).
	 * @param y_row The second row of the transform, (A10, A11, A12).
	 * @param scale The ratio of the extent of the volume to the extent of
	 * the slice.
	 */
	slice_frame(
		const volume_frequency &x_row,
		const volume_frequency &y_row,
		double scale
	) noexcept;

	/**
	 * @brief Get the step of the volume frequency per unit of k_x.
	 *
	 * @return const volume_frequency& The step X.
	 */
	const volume_frequency& get_x_step() const noexcept;

	/**
	 * @brief Get the step of the volume frequency per unit of k_y.
	 *
	 * @return const volume_frequency& The step Y.
	 */
	const volume_frequency& get_y_step() const noexcept;

	/**
	 * @brief Map a slice frequency to the volume.
	 *
	 * @param k_x The frequency along x, in samples.
	 * @param k_y The frequency along y, in samples.
	 * @return volume_frequency k_x X + k_y Y.
	 */
	volume_frequency map(double k_x, double k_y) const noexcept;

private:
	volume_frequency m_x_step;
	volume_frequency m_y_step;
};

} // namespace cpu
} // namespace rexlib
