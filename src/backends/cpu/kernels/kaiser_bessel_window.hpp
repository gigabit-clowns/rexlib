// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/em/ops/parameters/gridding_window.hpp>

#include <cstddef>
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
 * I0(beta sqrt(1 - (d / w)^2)) / I0(beta) for |d| < w, and 0 beyond. It is
 * read from a table of the window sampled at @ref samples_per_voxel points
 * per voxel, with linear interpolation between them.
 */
class kaiser_bessel_window
{
public:
	/**
	 * @brief Number of table entries per voxel of distance.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t samples_per_voxel = 1024;

	/**
	 * @brief Construct the window of a gridding_window.
	 *
	 * @param window The window. Its profile must be
	 * gridding_profile::kaiser_bessel.
	 */
	explicit kaiser_bessel_window(const em::gridding_window &window);

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
	 * @return double The half-width, in voxels.
	 */
	double get_half_width() const noexcept;

	/**
	 * @brief Evaluate the window at a distance.
	 *
	 * @param distance The signed distance from the sample, in voxels.
	 * @return double The weight. 0 at and beyond the half-width.
	 */
	double evaluate(double distance) const noexcept;

	/**
	 * @brief Place a sample on the voxels of an axis.
	 *
	 * @tparam T The type of the weights.
	 * @param coordinate The position of the sample along the axis, in
	 * voxels.
	 * @param weights Receives get_tap_count() weights, one per voxel from
	 * the first.
	 * @return std::ptrdiff_t The index of the first voxel, the lowest one
	 * within the half-width of the sample.
	 */
	template <typename T>
	std::ptrdiff_t place(double coordinate, T *weights) const noexcept;

private:
	double m_half_width;
	std::size_t m_tap_count;
	std::vector<double> m_table;
};

} // namespace cpu
} // namespace rexlib

#include "kaiser_bessel_window.inl"
