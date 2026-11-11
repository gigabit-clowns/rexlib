// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Window placing a sample on the voxels within its half-width, with
 * Kaiser-Bessel weights.
 *
 * The weight of a voxel at a distance d from the sample is
 * I0(beta sqrt(1 - (d / w)^2)) / I0(beta) for |d| < w, and 0 from w on. It
 * is read from a table of the window sampled at about
 * @ref samples_per_voxel points per voxel, a whole number of steps from 0
 * to w, with linear interpolation between them; the window does not
 * vanish at w, and the table ends with its value there.
 *
 * @tparam T The type of the weights.
 */
template <typename T>
class kaiser_bessel_window
{
public:
	/**
	 * @brief Least number of table entries per voxel of distance.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t samples_per_voxel = 1024;

	/**
	 * @brief Number of samples placed at once.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t block_size = 64;

	/**
	 * @brief Construct the window of a half-width and a shape.
	 *
	 * @param half_width The half-width w, in voxels. Positive.
	 * @param shape The shape parameter beta. From 0 to ln of the largest
	 * double.
	 */
	kaiser_bessel_window(double half_width, double shape);

	/**
	 * @brief Get the number of voxels a sample is placed on.
	 *
	 * @return std::size_t 2w rounded up, with w the half-width. The last
	 * voxel may receive a weight of 0.
	 */
	std::size_t get_tap_count() const noexcept;

	/**
	 * @brief Get the half-width of the window.
	 *
	 * @return T The half-width, in voxels.
	 */
	T get_half_width() const noexcept;

	/**
	 * @brief Evaluate the window at a distance.
	 *
	 * @param distance The signed distance from the sample, in voxels.
	 * @return T The weight. 0 at and beyond the half-width.
	 */
	T evaluate(T distance) const noexcept;

	/**
	 * @brief Place a run of samples on the voxels of an axis.
	 *
	 * @param coordinates The position of each sample along the axis, in
	 * voxels. Within the range of std::int32_t.
	 * @param count The number of samples.
	 * @param first Receives the index of the first voxel of each sample, the
	 * lowest one within the half-width of the sample.
	 * @param weights Receives get_tap_count() weights per sample, the
	 * weights of tap t from weights + t * weight_stride.
	 * @param weight_stride The distance between the weights of two taps.
	 */
	void place(
		const T *coordinates,
		std::int32_t count,
		std::int32_t *first,
		T *weights,
		std::size_t weight_stride
	) const noexcept;

private:
	T m_half_width;
	std::size_t m_tap_count;
	T m_scale;
	std::int32_t m_step_count;
	std::vector<T> m_table;
};

} // namespace cpu
} // namespace rexlib

#include "kaiser_bessel_window.inl"
