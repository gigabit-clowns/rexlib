// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <ostream>

namespace rexlib
{
namespace em
{

/**
 * @brief Shape of the one-dimensional window of a gridding_window.
 *
 * The window phi(d) weighs a voxel at a signed distance d from a sample, in
 * voxels, along one axis. It is 0 for |d| >= w, where w is the half-width
 * of the window.
 */
enum class gridding_profile
{
	/**
	 * @brief Weight 1 on the voxel nearest to the sample, 0 elsewhere.
	 *
	 * The half-width is 1/2. A sample halfway between two voxels goes to
	 * the one farther from 0.
	 */
	box,

	/**
	 * @brief Weight 1 - |d|.
	 *
	 * The half-width is 1, so a sample reaches the two voxels around it.
	 */
	triangle,

	/**
	 * @brief Weight I0(beta sqrt(1 - (d / w)^2)) / I0(beta).
	 *
	 * The half-width w and the shape beta are those of the window. I0 is
	 * the modified Bessel function of the first kind and order 0.
	 */
	kaiser_bessel
};

REXLIB_API
const char* to_string(gridding_profile profile) noexcept;

REXLIB_API
std::ostream& operator<<(std::ostream &os, gridding_profile profile);

} // namespace em
} // namespace rexlib
