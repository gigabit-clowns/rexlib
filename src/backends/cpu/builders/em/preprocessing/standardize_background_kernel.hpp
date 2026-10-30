// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "background_geometry.hpp"
#include "background_moments_kernel.hpp"
#include "standardization_map_kernel.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Batch reduce-map kernel of a background standardization.
 *
 * The first reduction computes the mean and the scale of the finite
 * background elements. The second computes them again over those whose
 * value, standardized with the first, lies within the bounds. The map
 * standardizes with the second, and replaces every element it does not
 * keep with a draw keyed by the seed of the image.
 *
 * Its statistics are the mean and the scale, in that order.
 */
class standardize_background_kernel
{
public:
	using statistics_type = std::array<double, 2>;

	/**
	 * @brief Construct the kernel.
	 *
	 * @param geometry The background of an image.
	 * @param lower_bound The lowest standardized value kept.
	 * @param upper_bound The highest standardized value kept.
	 */
	standardize_background_kernel(
		background_geometry geometry,
		double lower_bound,
		double upper_bound
	) noexcept;

	/**
	 * @brief Make the reduction of a pass.
	 *
	 * @param pass The pass, 0 or 1.
	 * @param previous The statistics of the pass before.
	 * @return background_moments_kernel The reduction. It refers to the
	 * geometry of this kernel.
	 */
	background_moments_kernel make_reduction(
		std::size_t pass,
		const statistics_type &previous
	) const noexcept;

	/**
	 * @brief Make the map of an image.
	 *
	 * @tparam Value The element type of the value.
	 * @param statistics The statistics of the last pass.
	 * @param inputs The value and the seed, placed at the image.
	 * @param counter The counter of the first element of the image.
	 * @return standardization_map_kernel The map.
	 */
	template <typename Value>
	standardization_map_kernel make_map(
		const statistics_type &statistics,
		const std::tuple<const Value*, const std::uint64_t*> &inputs,
		std::size_t counter
	) const noexcept;

private:
	background_geometry m_geometry;
	double m_lower_bound;
	double m_upper_bound;
};

} // namespace cpu
} // namespace rexlib

#include "standardize_background_kernel.inl"
