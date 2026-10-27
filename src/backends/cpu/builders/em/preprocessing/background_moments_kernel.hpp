// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "background_geometry.hpp"

#include <backends/cpu/loops/multidimensional_index.hpp>

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Reduction kernel computing the mean and the scale of the valid
 * background elements of an image.
 *
 * An element is valid when it is finite, lies in the background, and its
 * standardized value under a previous mean and scale lies within bounds.
 * The kernel counts the valid elements and sums them and their squares in
 * double, and finalizes to their mean and to the reciprocal of their
 * population standard deviation. Without a valid element the mean is 0 and
 * the scale 1; with a standard deviation of 0 the scale is 1.
 */
class background_moments_kernel
{
public:
	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<double, double, double>;
	};

	static REXLIB_CONST_CONSTEXPR bool reassociable_fold = true;

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
		double &count,
		double &sum,
		double &sum_of_squares,
		const T *value,
		const multidimensional_index &index
	) const noexcept;

	template <typename T>
	void combine(
		double &count,
		double &sum,
		double &sum_of_squares,
		const T *value,
		const multidimensional_index &index
	) const noexcept;

	void merge(
		double &count,
		double &sum,
		double &sum_of_squares,
		const double &other_count,
		const double &other_sum,
		const double &other_sum_of_squares
	) const noexcept;

	void finalize(
		double *mean,
		double *scale,
		const double &count,
		const double &sum,
		const double &sum_of_squares,
		std::size_t element_count
	) const noexcept;

private:
	bool is_valid(
		double value,
		const multidimensional_index &index
	) const noexcept;

	const background_geometry *m_geometry;
	double m_mean;
	double m_scale;
	double m_lower_bound;
	double m_upper_bound;
};

} // namespace cpu
} // namespace rexlib

#include "background_moments_kernel.inl"
