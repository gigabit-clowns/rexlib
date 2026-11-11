// SPDX-License-Identifier: GPL-3.0-only

#include "volume_tiling.hpp"

#include <rexlib/core/platform/assert.hpp>

#include <algorithm>
#include <cmath>

namespace rexlib
{
namespace cpu
{

namespace
{

std::size_t largest_power_of_two_within(std::size_t value) noexcept
{
	std::size_t result = 1;
	while (result * 2 <= value)
	{
		result *= 2;
	}
	return result;
}

std::size_t smallest_power_of_two_from(double value) noexcept
{
	std::size_t result = 1;
	while (static_cast<double>(result) < value)
	{
		result *= 2;
	}
	return result;
}

std::size_t divide_rounding_up(std::size_t a, std::size_t b) noexcept
{
	return (a + b - 1) / b;
}

} // anonymous namespace

std::size_t choose_tile_extent(
	std::size_t extent,
	std::size_t row_bytes,
	std::size_t byte_budget,
	std::size_t task_floor,
	double half_width
) noexcept
{
	const auto largest = largest_power_of_two_within(extent);
	auto tile = largest;
	while (tile > 1)
	{
		const auto tiles_per_axis = divide_rounding_up(extent, tile);
		const bool within_budget = tile * tile * row_bytes <= byte_budget;
		const bool enough_tiles = tiles_per_axis * tiles_per_axis >= task_floor;
		if (within_budget && enough_tiles)
		{
			break;
		}
		tile /= 2;
	}

	const auto least = smallest_power_of_two_from(8.0 * half_width);
	return std::min(std::max(tile, least), largest);
}

volume_tiling::volume_tiling(
	std::size_t extent,
	std::size_t tile_extent
) noexcept
	: m_extent(extent)
	, m_tile_extent(tile_extent)
	, m_tiles_per_axis(divide_rounding_up(extent, tile_extent))
{
	REXLIB_ASSERT(extent > 0 && tile_extent > 0);
}

std::size_t volume_tiling::get_tile_count() const noexcept
{
	return m_tiles_per_axis * m_tiles_per_axis;
}

volume_tile volume_tiling::get_tile(std::size_t index) const noexcept
{
	REXLIB_ASSERT(index < get_tile_count());

	const auto z = index / m_tiles_per_axis;
	const auto y = index % m_tiles_per_axis;
	const auto z_begin = z * m_tile_extent;
	const auto y_begin = y * m_tile_extent;
	return volume_tile(
		m_extent,
		z_begin,
		std::min(z_begin + m_tile_extent, m_extent),
		y_begin,
		std::min(y_begin + m_tile_extent, m_extent)
	);
}

} // namespace cpu
} // namespace rexlib
