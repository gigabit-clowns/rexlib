// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>
#include <cstdint>
#include <tuple>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Vector kernel standardizing a run of elements, and replacing
 * those it does not keep with standard normal draws.
 *
 * An element is kept when its standardized value is finite and lies within
 * the bounds. The run is first standardized whole; only a run holding an
 * element that is not kept is walked again, to replace that element with
 * the draw of its counter.
 */
class standardization_map_kernel
{
public:
	/**
	 * @brief Construct the map of one image.
	 *
	 * @param mean The mean subtracted.
	 * @param scale The factor applied after subtracting the mean.
	 * @param lower_bound The lowest standardized value kept.
	 * @param upper_bound The highest standardized value kept.
	 * @param seed The seed of the draws.
	 * @param counter The counter of the first element of the image, added to
	 * the counter of every element within it.
	 */
	standardization_map_kernel(
		double mean,
		double scale,
		double lower_bound,
		double upper_bound,
		std::uint64_t seed,
		std::uint64_t counter
	) noexcept;

	/**
	 * @brief Standardize a run.
	 *
	 * @param pointers The first result element and the first value of the
	 * run.
	 * @param strides The strides of the result and of the value, in
	 * elements.
	 * @param count The number of elements of the run.
	 * @param index The counter of the run within the image.
	 */
	template <
		typename Out,
		typename In,
		typename OutStride,
		typename InStride,
		typename IndexRun
	>
	void operator()(
		const std::tuple<Out*, const In*> &pointers,
		const std::tuple<OutStride, InStride> &strides,
		std::size_t count,
		const IndexRun &index
	) const noexcept;

private:
	template <typename T>
	bool is_kept(T standardized, T lower_bound, T upper_bound) const noexcept;

	double m_mean;
	double m_scale;
	double m_lower_bound;
	double m_upper_bound;
	std::uint64_t m_seed;
	std::uint64_t m_counter;
};

} // namespace cpu
} // namespace rexlib

#include "standardization_map_kernel.inl"
