// SPDX-License-Identifier: GPL-3.0-only

#include "background_geometry.hpp"

#include <cmath>

namespace rexlib
{
namespace cpu
{

inline
background_geometry::background_geometry(
	span<const std::size_t> extents,
	double radius
)
	: m_extents(extents.begin(), extents.end())
	, m_squared_radius(radius * radius)
{
	m_centre.reserve(extents.size());
	for (const auto extent : extents)
	{
		m_centre.push_back(static_cast<std::ptrdiff_t>(extent / 2));
	}
}

inline
span<const std::ptrdiff_t> background_geometry::get_centre() const noexcept
{
	return make_span(m_centre);
}

inline
double background_geometry::get_squared_radius() const noexcept
{
	return m_squared_radius;
}

inline
bool background_geometry::is_background(
	const multidimensional_index &index
) const noexcept
{
	const auto squared_distance =
		compute_squared_distance(index, m_centre.size());
	return static_cast<double>(squared_distance) > m_squared_radius;
}

inline
std::pair<std::size_t, std::size_t>
background_geometry::find_excluded_range(
	const multidimensional_index &index,
	std::size_t count
) const noexcept
{
	const auto axis = index.get_inner_axis();
	if (axis >= m_centre.size())
	{
		return is_background(index)
			? std::make_pair(std::size_t(0), std::size_t(0))
			: std::make_pair(std::size_t(0), count);
	}

	const auto squared_distance = compute_squared_distance(index, axis);
	if (static_cast<double>(squared_distance) > m_squared_radius)
	{
		return std::make_pair(std::size_t(0), std::size_t(0));
	}

	const auto half_width = static_cast<std::ptrdiff_t>(
		compute_half_width(squared_distance, m_extents[axis])
	);
	const auto start = static_cast<std::ptrdiff_t>(index[axis]);
	const auto length = static_cast<std::ptrdiff_t>(count);
	const auto clamp = [length](std::ptrdiff_t position)
	{
		return static_cast<std::size_t>(
			position < 0 ? 0 : (position > length ? length : position)
		);
	};

	return std::make_pair(
		clamp(m_centre[axis] - half_width - start),
		clamp(m_centre[axis] + half_width + 1 - start)
	);
}

inline
std::ptrdiff_t background_geometry::compute_squared_distance(
	const multidimensional_index &index,
	std::size_t skipped_axis
) const noexcept
{
	std::ptrdiff_t result = 0;
	for (std::size_t axis = 0; axis < m_centre.size(); ++axis)
	{
		const auto distance =
			static_cast<std::ptrdiff_t>(index[axis]) - m_centre[axis];
		result += axis == skipped_axis ? 0 : distance * distance;
	}
	return result;
}

inline
std::size_t background_geometry::compute_half_width(
	std::ptrdiff_t squared_distance,
	std::size_t extent
) const noexcept
{
	const auto is_within = [this, squared_distance](std::ptrdiff_t width)
	{
		return
			static_cast<double>(squared_distance + width * width) <=
			m_squared_radius;
	};

	const auto widest = static_cast<std::ptrdiff_t>(extent);
	if (is_within(widest))
	{
		return extent;
	}

	auto result = static_cast<std::ptrdiff_t>(std::sqrt(
		m_squared_radius - static_cast<double>(squared_distance)
	));
	result = result < widest ? result : widest;
	while (result > 0 && !is_within(result))
	{
		--result;
	}
	while (result < widest && is_within(result + 1))
	{
		++result;
	}
	return static_cast<std::size_t>(result);
}

} // namespace cpu
} // namespace rexlib
