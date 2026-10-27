// SPDX-License-Identifier: GPL-3.0-only

#include "standardize_background_kernel.hpp"

#include <limits>
#include <utility>

namespace rexlib
{
namespace cpu
{

inline
standardize_background_kernel::standardize_background_kernel(
	background_geometry geometry,
	double lower_bound,
	double upper_bound
) noexcept
	: m_geometry(std::move(geometry))
	, m_lower_bound(lower_bound)
	, m_upper_bound(upper_bound)
{
}

inline
background_moments_kernel standardize_background_kernel::make_reduction(
	std::size_t pass,
	const statistics_type &previous
) const noexcept
{
	if (pass == 0)
	{
		const auto infinity = std::numeric_limits<double>::infinity();
		return background_moments_kernel(
			m_geometry,
			0.0,
			1.0,
			-infinity,
			infinity
		);
	}

	return background_moments_kernel(
		m_geometry,
		previous[0],
		previous[1],
		m_lower_bound,
		m_upper_bound
	);
}

template <typename Value>
inline
standardization_map_kernel standardize_background_kernel::make_map(
	const statistics_type &statistics,
	const std::tuple<const Value*, const std::uint64_t*> &inputs,
	std::size_t counter
) const noexcept
{
	return standardization_map_kernel(
		statistics[0],
		statistics[1],
		m_lower_bound,
		m_upper_bound,
		*std::get<1>(inputs),
		counter
	);
}

} // namespace cpu
} // namespace rexlib
