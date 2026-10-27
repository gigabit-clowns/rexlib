// SPDX-License-Identifier: GPL-3.0-only

#include "background_moments_kernel.hpp"

#include <backends/cpu/load_store.hpp>

#include <cmath>

namespace rexlib
{
namespace cpu
{

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
	double &count,
	double &sum,
	double &sum_of_squares,
	const T *value,
	const multidimensional_index &index
) const noexcept
{
	count = 0.0;
	sum = 0.0;
	sum_of_squares = 0.0;
	combine(count, sum, sum_of_squares, value, index);
}

template <typename T>
inline
void background_moments_kernel::combine(
	double &count,
	double &sum,
	double &sum_of_squares,
	const T *value,
	const multidimensional_index &index
) const noexcept
{
	const auto x = static_cast<double>(load(value));
	const auto valid = is_valid(x, index);
	const auto kept = valid ? x : 0.0;

	count += valid ? 1.0 : 0.0;
	sum += kept;
	sum_of_squares += kept * kept;
}

inline
void background_moments_kernel::merge(
	double &count,
	double &sum,
	double &sum_of_squares,
	const double &other_count,
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
	const double &count,
	const double &sum,
	const double &sum_of_squares,
	std::size_t /*element_count*/
) const noexcept
{
	if (count == 0.0)
	{
		*mean = 0.0;
		*scale = 1.0;
		return;
	}

	*mean = sum / count;
	const auto variance = sum_of_squares / count - *mean * *mean;
	*scale = variance > 0.0 ? 1.0 / std::sqrt(variance) : 1.0;
}

inline
bool background_moments_kernel::is_valid(
	double value,
	const multidimensional_index &index
) const noexcept
{
	const auto standardized = (value - m_mean) * m_scale;
	return
		std::isfinite(value) &
		m_geometry->is_background(index) &
		(standardized >= m_lower_bound) &
		(standardized <= m_upper_bound);
}

} // namespace cpu
} // namespace rexlib
