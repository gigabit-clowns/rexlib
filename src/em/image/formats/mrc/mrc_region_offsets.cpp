// SPDX-License-Identifier: GPL-3.0-only

#include "mrc_region_offsets.hpp"

#include <rexlib/em/image/image_transfer_plan.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace em
{
namespace mrc
{

namespace
{

void check_rank(
	std::size_t actual,
	std::size_t expected,
	const char *message
)
{
	if (actual != expected)
	{
		throw std::invalid_argument(message);
	}
}

// The extents of a batch cover the trailing axes of a side, which spans a
// single position along the leading ones, so the extent of an axis is
// resolved through get_region_extent rather than by indexing them directly.
std::ptrdiff_t resolve_offset(
	const image_transfer_plan &regions,
	span<const std::size_t> region_offset,
	span<const std::size_t> extents,
	span<const std::ptrdiff_t> strides
)
{
	const auto rank = extents.size();

	std::ptrdiff_t offset = 0;
	for (std::size_t axis = 0; axis < rank; ++axis)
	{
		// Check "offset within bounds, then extent within what remains"
		// to avoid false negatives due to overflow
		const auto extent = get_region_extent(regions, rank, axis);
		const auto position = region_offset[axis];
		const auto boundary = extents[axis];
		const auto remaining = boundary - position;
		if (position > boundary || extent > remaining)
		{
			throw std::out_of_range(
				"mrc_region_offsets: A region does not fit where it is "
				"placed."
			);
		}

		offset += static_cast<std::ptrdiff_t>(region_offset[axis]) *
			strides[axis];
	}

	return offset;
}

} // anonymous namespace

mrc_region_offsets::mrc_region_offsets(
	const image_transfer_plan &regions,
	span<const std::size_t> file_extents,
	span<const std::ptrdiff_t> file_strides,
	span<const std::size_t> array_extents,
	span<const std::ptrdiff_t> array_strides,
	std::ptrdiff_t array_offset
)
{
	check_rank(
		file_extents.size(),
		regions.get_file_rank(),
		"mrc_region_offsets: The file extents do not have the file rank of "
		"the batch."
	);
	check_rank(
		array_extents.size(),
		regions.get_array_rank(),
		"mrc_region_offsets: The array extents do not have the array rank "
		"of the batch."
	);
	check_rank(
		file_strides.size(),
		file_extents.size(),
		"mrc_region_offsets: The file strides do not have the rank of its "
		"extents."
	);
	check_rank(
		array_strides.size(),
		array_extents.size(),
		"mrc_region_offsets: The array strides do not have the rank of its "
		"extents."
	);

	const auto count = regions.get_region_count();
	std::vector<std::pair<std::ptrdiff_t, std::ptrdiff_t>> resolved;
	resolved.reserve(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		resolved.emplace_back(
			resolve_offset(
				regions,
				regions.get_file_offset(i),
				file_extents,
				file_strides
			),
			array_offset +
			resolve_offset(
				regions,
				regions.get_array_offset(i),
				array_extents,
				array_strides
			)
		);
	}

	if (!std::is_sorted(resolved.begin(), resolved.end()))
	{
		std::sort(resolved.begin(), resolved.end());
	}

	m_array.reserve(count);
	m_file.reserve(count);
	for (const auto &pair : resolved)
	{
		m_file.push_back(pair.first);
		m_array.push_back(pair.second);
	}
}

std::size_t mrc_region_offsets::get_region_count() const noexcept
{
	return m_array.size();
}

span<const std::ptrdiff_t> mrc_region_offsets::get_array() const noexcept
{
	return make_span(m_array.data(), m_array.size());
}

span<const std::ptrdiff_t> mrc_region_offsets::get_file() const noexcept
{
	return make_span(m_file.data(), m_file.size());
}

} // namespace mrc
} // namespace em
} // namespace rexlib
