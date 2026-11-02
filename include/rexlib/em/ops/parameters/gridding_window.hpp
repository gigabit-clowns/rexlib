// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/ops/parameters/gridding_profile.hpp>

namespace rexlib
{
namespace em
{

/**
 * @brief Separable window that spreads a sample over the voxels of a grid.
 *
 * A sample at the continuous position K adds to the voxel g with the
 * weight phi(g_x - K_x) phi(g_y - K_y) phi(g_z - K_z), where phi is the
 * one-dimensional window of the profile. A voxel at a distance of the
 * half-width or more along any axis receives nothing.
 *
 * @see gridding_profile
 */
class gridding_window
{
public:
	/**
	 * @brief Get the window of nearest-neighbour gridding.
	 *
	 * @return gridding_window A box of half-width 1/2.
	 */
	REXLIB_API
	static gridding_window nearest() noexcept;

	/**
	 * @brief Get the window of trilinear gridding.
	 *
	 * @return gridding_window A triangle of half-width 1.
	 */
	REXLIB_API
	static gridding_window linear() noexcept;

	/**
	 * @brief Get a Kaiser-Bessel window.
	 *
	 * @param half_width The half-width w of the window, in voxels.
	 * @param shape The shape beta of the window. 0 gives a box of
	 * half-width w.
	 * @return gridding_window The window.
	 *
	 * @throws std::invalid_argument When the half-width is not finite and
	 * positive, or the shape is not finite and 0 or more.
	 */
	REXLIB_API
	static gridding_window kaiser_bessel(double half_width, double shape);

	/**
	 * @brief Get the profile of the window.
	 *
	 * @return gridding_profile The profile.
	 */
	REXLIB_API
	gridding_profile get_profile() const noexcept;

	/**
	 * @brief Get the half-width of the window.
	 *
	 * @return double The half-width, in voxels.
	 */
	REXLIB_API
	double get_half_width() const noexcept;

	/**
	 * @brief Get the shape of a Kaiser-Bessel window.
	 *
	 * @return double The shape beta. 0 for the other profiles.
	 */
	REXLIB_API
	double get_shape() const noexcept;

private:
	gridding_window(
		gridding_profile profile,
		double half_width,
		double shape
	) noexcept;

	gridding_profile m_profile;
	double m_half_width;
	double m_shape;
};

} // namespace em
} // namespace rexlib
