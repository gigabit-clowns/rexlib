// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "background_geometry.hpp"

#include <backends/cpu/loops/multidimensional_index.hpp>

#include <rexlib/core/meta/type_list.hpp>

#include <cstddef>
#include <tuple>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Vector reduction kernel computing the mean and the scale of the
 * valid background elements of an image.
 *
 * An element is valid when it is finite, lies in the background, and its
 * standardized value under a previous mean and scale lies within bounds.
 * The kernel counts the valid elements and sums them and their squares in
 * double, and finalizes to their mean and to the reciprocal of their
 * population standard deviation. Without a valid element the mean is 0 and
 * the scale 1; with a standard deviation of 0 the scale is 1.
 *
 * A run is folded without testing its elements against the background one
 * by one: the elements that are not background form one range of it, which
 * is skipped, and the rest is folded over several lanes.
 */
class background_moments_kernel
{
public:
	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<std::size_t, double, double>;
	};

	/**
	 * @brief Construct the kernel of one pass.
	 *
	 * @param geometry The background. It must outlive the kernel.
	 * @param mean The previous mean.
	 * @param scale The previous scale.
	 * @param lower_bound The lowest standardized value of a valid element.
	 * @param upper_bound The highest standardized value of a valid element.
	 */
	background_moments_kernel(
		const background_geometry &geometry,
		double mean,
		double scale,
		double lower_bound,
		double upper_bound
	) noexcept;

	template <typename T>
	void seed(
		std::size_t &count,
		double &sum,
		double &sum_of_squares,
		const T *value,
		const multidimensional_index &index
	) const noexcept;

	template <typename T, typename Stride>
	void combine_run(
		const std::tuple<std::size_t*, double*, double*> &moments,
		const std::tuple<const T*> &values,
		const std::tuple<Stride> &strides,
		std::size_t count,
		const multidimensional_index &index
	) const noexcept;

	template <typename T, typename KeptStride, typename ReducedStride>
	void combine_strip(
		const std::tuple<std::size_t*, double*, double*> &moments,
		const std::tuple<const T*> &values,
		const std::tuple<KeptStride> &kept_strides,
		const std::tuple<ReducedStride> &reduced_strides,
		std::size_t width,
		std::size_t count,
		const multidimensional_index &index
	) const noexcept;

	void merge(
		std::size_t &count,
		double &sum,
		double &sum_of_squares,
		const std::size_t &other_count,
		const double &other_sum,
		const double &other_sum_of_squares
	) const noexcept;

	void finalize(
		double *mean,
		double *scale,
		const std::size_t &count,
		const double &sum,
		const double &sum_of_squares,
		std::size_t element_count
	) const noexcept;

private:
	template <typename T, typename Stride>
	void fold(
		std::size_t &count,
		double &sum,
		double &sum_of_squares,
		const T *value,
		Stride stride,
		std::size_t first,
		std::size_t last
	) const noexcept;

	bool is_valid(double value) const noexcept;

	const background_geometry *m_geometry;
	double m_mean;
	double m_scale;
	double m_lower_bound;
	double m_upper_bound;
};

} // namespace cpu
} // namespace rexlib

#include "background_moments_kernel.inl"
