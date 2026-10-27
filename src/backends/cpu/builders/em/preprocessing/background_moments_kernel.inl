// SPDX-License-Identifier: GPL-3.0-only

#include "background_moments_kernel.hpp"

#include <backends/cpu/load_store.hpp>

#include <rexlib/core/platform/constexpr.hpp>

#include <array>
#include <cmath>
#include <limits>

namespace rexlib
{
namespace cpu
{

namespace detail
{

REXLIB_CONST_CONSTEXPR std::size_t background_moment_lane_count = 4;

} // namespace detail

inline
background_moments_kernel::background_moments_kernel(
	const background_geometry &geometry,
	double mean,
	double scale,
	double lower_bound,
	double upper_bound
) noexcept
	: m_geometry(&geometry)
	, m_mean(mean)
	, m_scale(scale)
	, m_lower_bound(lower_bound)
	, m_upper_bound(upper_bound)
{
}

template <typename T>
inline
void background_moments_kernel::seed(
	std::size_t &count,
	double &sum,
	double &sum_of_squares,
	const T *value,
	const multidimensional_index &index
) const noexcept
{
	const auto x = static_cast<double>(load(value));
	const auto valid = is_valid(x) && m_geometry->is_background(index);
	count = valid ? 1 : 0;
	sum = valid ? x : 0.0;
	sum_of_squares = valid ? x * x : 0.0;
}

template <typename T, typename Stride>
inline
void background_moments_kernel::combine_run(
	const std::tuple<std::size_t*, double*, double*> &moments,
	const std::tuple<const T*> &values,
	const std::tuple<Stride> &strides,
	std::size_t count,
	const multidimensional_index &index
) const noexcept
{
	const auto excluded = m_geometry->find_excluded_range(index, count);
	auto &total = *std::get<0>(moments);
	auto &sum = *std::get<1>(moments);
	auto &sum_of_squares = *std::get<2>(moments);
	const auto *value = std::get<0>(values);
	const auto stride = std::get<0>(strides);

	fold(total, sum, sum_of_squares, value, stride, 0, excluded.first);
	fold(total, sum, sum_of_squares, value, stride, excluded.second, count);
}

template <typename T, typename KeptStride, typename ReducedStride>
inline
void background_moments_kernel::combine_strip(
	const std::tuple<std::size_t*, double*, double*> &moments,
	const std::tuple<const T*> &values,
	const std::tuple<KeptStride> &kept_strides,
	const std::tuple<ReducedStride> &reduced_strides,
	std::size_t width,
	std::size_t count,
	const multidimensional_index &index
) const noexcept
{
	const std::ptrdiff_t kept_stride = std::get<0>(kept_strides);
	for (std::size_t j = 0; j < width; ++j)
	{
		const auto offset = static_cast<std::ptrdiff_t>(j);
		combine_run(
			std::make_tuple(
				std::get<0>(moments) + offset,
				std::get<1>(moments) + offset,
				std::get<2>(moments) + offset
			),
			std::make_tuple(std::get<0>(values) + offset * kept_stride),
			reduced_strides,
			count,
			index
		);
	}
}

inline
void background_moments_kernel::merge(
	std::size_t &count,
	double &sum,
	double &sum_of_squares,
	const std::size_t &other_count,
	const double &other_sum,
	const double &other_sum_of_squares
) const noexcept
{
	count += other_count;
	sum += other_sum;
	sum_of_squares += other_sum_of_squares;
}

inline
void background_moments_kernel::finalize(
	double *mean,
	double *scale,
	const std::size_t &count,
	const double &sum,
	const double &sum_of_squares,
	std::size_t /*element_count*/
) const noexcept
{
	if (count == 0)
	{
		*mean = 0.0;
		*scale = 1.0;
		return;
	}

	const auto total = static_cast<double>(count);
	*mean = sum / total;
	const auto variance = sum_of_squares / total - *mean * *mean;
	*scale = variance > 0.0 ? 1.0 / std::sqrt(variance) : 1.0;
}

template <typename T, typename Stride>
inline
void background_moments_kernel::fold(
	std::size_t &count,
	double &sum,
	double &sum_of_squares,
	const T *value,
	Stride stride,
	std::size_t first,
	std::size_t last
) const noexcept
{
	using count_lanes_type =
		std::array<std::size_t, detail::background_moment_lane_count>;
	using sum_lanes_type =
		std::array<double, detail::background_moment_lane_count>;

	count_lanes_type counts = {};
	sum_lanes_type sums = {};
	sum_lanes_type squares = {};

	const auto step = static_cast<std::ptrdiff_t>(stride);
	auto position = first;
	for (
		;
		position + counts.size() <= last;
		position += counts.size()
	)
	{
		for (std::size_t lane = 0; lane < counts.size(); ++lane)
		{
			const auto element =
				static_cast<std::ptrdiff_t>(position + lane) * step;
			const auto x = static_cast<double>(load(value + element));
			const auto valid = is_valid(x);
			const auto kept = valid ? x : 0.0;
			counts[lane] += valid ? 1 : 0;
			sums[lane] += kept;
			squares[lane] += kept * kept;
		}
	}
	for (; position < last; ++position)
	{
		const auto element = static_cast<std::ptrdiff_t>(position) * step;
		const auto x = static_cast<double>(load(value + element));
		const auto valid = is_valid(x);
		const auto kept = valid ? x : 0.0;
		counts[0] += valid ? 1 : 0;
		sums[0] += kept;
		squares[0] += kept * kept;
	}

	for (std::size_t lane = 0; lane < counts.size(); ++lane)
	{
		count += counts[lane];
		sum += sums[lane];
		sum_of_squares += squares[lane];
	}
}

inline
bool background_moments_kernel::is_valid(double value) const noexcept
{
	const auto standardized = (value - m_mean) * m_scale;
	return
		(std::abs(value) <= std::numeric_limits<double>::max()) &
		(standardized >= m_lower_bound) &
		(standardized <= m_upper_bound);
}

} // namespace cpu
} // namespace rexlib
