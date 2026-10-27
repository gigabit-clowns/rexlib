// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/multidimensional_index.hpp>

#include <rexlib/core/span.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The background of an image: its elements farther from its centre
 * than a radius.
 *
 * The centre lies at index n / 2, rounded down, along each axis of extent
 * n. Distances are measured in elements, the same along every axis.
 */
class background_geometry
{
public:
	/**
	 * @brief Construct the background of images of some extents.
	 *
	 * @param extents The extents of an image.
	 * @param radius The radius, in elements. Elements at that distance from
	 * the centre or nearer are not background.
	 */
	background_geometry(span<const std::size_t> extents, double radius);

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
	 * @brief Check whether an element is background.
	 *
	 * @param index The coordinates of the element, of the rank of the
	 * image.
	 * @return true The element is farther from the centre than the radius.
	 * @return false The element is at the radius or nearer.
	 */
	bool is_background(const multidimensional_index &index) const noexcept;

private:
	std::vector<std::ptrdiff_t> m_centre;
	double m_squared_radius;
};

} // namespace cpu
} // namespace rexlib

#include "background_geometry.inl"
