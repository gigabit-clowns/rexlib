// SPDX-License-Identifier: GPL-3.0-only

#include "volume_tile.hpp"

#include <rexlib/core/platform/assert.hpp>

#include <algorithm>

namespace rexlib
{
namespace cpu
{

volume_tile::volume_tile(
	std::size_t extent,
	std::size_t z_begin,
	std::size_t z_end,
	std::size_t y_begin,
	std::size_t y_end
) noexcept
	: m_extent(extent)
	, m_z_begin(z_begin)
	, m_z_end(z_end)
	, m_y_begin(y_begin)
	, m_y_end(y_end)
{
}

std::size_t volume_tile::get_extent() const noexcept
{
	return m_extent;
}

std::size_t volume_tile::get_z_begin() const noexcept
{
	return m_z_begin;
}

std::size_t volume_tile::get_z_end() const noexcept
{
	return m_z_end;
}

std::size_t volume_tile::get_y_begin() const noexcept
{
	return m_y_begin;
}

std::size_t volume_tile::get_y_end() const noexcept
{
	return m_y_end;
}

std::size_t volume_tile::list_z_frequencies(
	interval_list_type &intervals
) const noexcept
{
	return list_stored_frequencies(m_extent, m_z_begin, m_z_end, intervals);
}

std::size_t volume_tile::list_y_frequencies(
	interval_list_type &intervals
) const noexcept
{
	return list_stored_frequencies(m_extent, m_y_begin, m_y_end, intervals);
}

std::size_t list_stored_frequencies(
	std::size_t extent,
	std::size_t begin,
	std::size_t end,
	volume_tile::interval_list_type &intervals
) noexcept
{
	REXLIB_ASSERT(begin < end && end <= extent);

	const auto half = static_cast<std::ptrdiff_t>(extent / 2);
	const auto signed_extent = static_cast<std::ptrdiff_t>(extent);
	const auto first = static_cast<std::ptrdiff_t>(begin);
	const auto last = static_cast<std::ptrdiff_t>(end) - 1;
	const bool holds_both_halves =
		extent % 2 == 0 && first <= half && half <= last;

	std::size_t count = 0;
	if (last > half)
	{
		const auto lower = std::max(first, half + 1) - signed_extent;
		intervals[count++] = frequency_interval(
			holds_both_halves ? -half : lower,
			last - signed_extent
		);
	}
	else if (holds_both_halves)
	{
		intervals[count++] = frequency_interval(-half, -half);
	}

	if (first <= half)
	{
		intervals[count++] = frequency_interval(first, std::min(last, half));
	}

	return count;
}

} // namespace cpu
} // namespace rexlib
