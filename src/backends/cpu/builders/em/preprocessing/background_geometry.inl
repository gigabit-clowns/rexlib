// SPDX-License-Identifier: GPL-3.0-only

#include "background_geometry.hpp"

namespace rexlib
{
namespace cpu
{

inline
background_geometry::background_geometry(
	span<const std::size_t> extents,
	double radius
)
	: m_squared_radius(radius * radius)
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
	std::ptrdiff_t squared_distance = 0;
	for (std::size_t axis = 0; axis < m_centre.size(); ++axis)
	{
		const auto distance =
			static_cast<std::ptrdiff_t>(index[axis]) - m_centre[axis];
		squared_distance += distance * distance;
	}
	return static_cast<double>(squared_distance) > m_squared_radius;
}

} // namespace cpu
} // namespace rexlib
