// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/indices/multidimensional_index.hpp>

#include <rexlib/core/span.hpp>

#include <cstddef>
#include <utility>
#include <vector>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The elements of an array within a radius of its centre.
 *
 * The centre lies at index n / 2, rounded down, along each axis of extent
 * n. Distances are measured in elements, the same along every axis.
 */
class centred_ball
{
public:
	/**
	 * @brief Construct the ball of arrays of some extents.
	 *
	 * @param extents The extents of an array.
	 * @param radius The radius, in elements. Elements at that distance from
	 * the centre or nearer are inside.
	 */
	centred_ball(span<const std::size_t> extents, double radius);

	/**
	 * @brief Get the index of the centre along every axis.
	 *
	 * @return span<const std::ptrdiff_t> One index per axis.
	 */
	span<const std::ptrdiff_t> get_centre() const noexcept;

	/**
	 * @brief Get the square of the radius.
	 *
	 * @return double The squared radius, in elements.
	 */
	double get_squared_radius() const noexcept;

	/**
	 * @brief Check whether an element is outside the ball.
	 *
	 * @param index The coordinates of the element, of the rank of the
	 * array.
	 * @return true The element is farther from the centre than the radius.
	 * @return false The element is at the radius or nearer.
	 */
	bool is_outside(const multidimensional_index &index) const noexcept;

	/**
	 * @brief Find the elements of a run that are inside the ball.
	 *
	 * A run holds consecutive elements along the inner axis of @p index,
	 * so the elements inside form one range within it.
	 *
	 * @param index The coordinates of the first element of the run.
	 * @param count The number of elements of the run.
	 * @return std::pair<std::size_t, std::size_t> The positions within the
	 * run of the first element inside and of the one past the last. They
	 * are equal when every element is outside.
	 */
	std::pair<std::size_t, std::size_t> find_inside_range(
		const multidimensional_index &index,
		std::size_t count
	) const noexcept;

private:
	std::ptrdiff_t compute_squared_distance(
		const multidimensional_index &index,
		std::size_t skipped_axis
	) const noexcept;

	std::size_t compute_half_width(
		std::ptrdiff_t squared_distance,
		std::size_t extent
	) const noexcept;

	std::vector<std::size_t> m_extents;
	std::vector<std::ptrdiff_t> m_centre;
	double m_squared_radius;
};

} // namespace cpu
} // namespace rexlib

#include "centred_ball.inl"
