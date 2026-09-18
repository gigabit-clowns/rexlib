// SPDX-License-Identifier: GPL-3.0-only

#include "image_region_clipping.hpp"

#include <rexlib/em/image/image_transfer_plan.hpp>

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace rexlib
{
namespace em
{

namespace
{

const std::size_t no_group = std::numeric_limits<std::size_t>::max();

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

std::size_t get_remaining(
	span<const std::size_t> extents,
	span<const std::size_t> offset,
	std::size_t axis
) noexcept
{
	const auto boundary = extents[axis];
	const auto position = offset[axis];
	return position < boundary ? boundary - position : 0;
}

// The extents of a plan cover the trailing axes of a side, which spans a
// single position along the leading ones. Those leading axes cannot be
// shortened, so a region that starts past one of them transfers nothing.
bool clip_region(
	const image_transfer_plan &regions,
	std::size_t region_index,
	span<const std::size_t> file_extents,
	span<const std::size_t> array_extents,
	std::vector<std::size_t> &extents
)
{
	const auto rank = regions.get_rank();
	const auto file_offset = regions.get_file_offset(region_index);
	const auto array_offset = regions.get_array_offset(region_index);
	const auto file_leading = regions.get_file_rank() - rank;
	const auto array_leading = regions.get_array_rank() - rank;

	for (std::size_t axis = 0; axis < file_leading; ++axis)
	{
		if (get_remaining(file_extents, file_offset, axis) == 0)
		{
			return false;
		}
	}

	for (std::size_t axis = 0; axis < array_leading; ++axis)
	{
		if (get_remaining(array_extents, array_offset, axis) == 0)
		{
			return false;
		}
	}

	const auto whole = regions.get_extents();
	extents.assign(whole.begin(), whole.end());
	for (std::size_t axis = 0; axis < rank; ++axis)
	{
		auto &extent = extents[axis];
		extent = std::min(
			extent,
			get_remaining(file_extents, file_offset, file_leading + axis)
		);
		extent = std::min(
			extent,
			get_remaining(array_extents, array_offset, array_leading + axis)
		);

		if (extent == 0)
		{
			return false;
		}
	}

	return true;
}

std::size_t find_group(
	const std::vector<std::vector<std::size_t>> &group_extents,
	const std::vector<std::size_t> &extents
) noexcept
{
	for (std::size_t group = 0; group < group_extents.size(); ++group)
	{
		if (group_extents[group] == extents)
		{
			return group;
		}
	}

	return group_extents.size();
}

} // anonymous namespace

bool make_clipped_transfer_plans(
	const image_transfer_plan &regions,
	span<const std::size_t> file_extents,
	span<const std::size_t> array_extents,
	std::vector<image_transfer_plan> &result
)
{
	check_rank(
		file_extents.size(),
		regions.get_file_rank(),
		"make_clipped_transfer_plans: The file extents do not have the file "
		"rank of the regions."
	);
	check_rank(
		array_extents.size(),
		regions.get_array_rank(),
		"make_clipped_transfer_plans: The array extents do not have the "
		"array rank of the regions."
	);

	result.clear();

	const auto count = regions.get_region_count();
	const auto whole = regions.get_extents();

	std::vector<std::size_t> group_of_region(count, no_group);
	std::vector<std::vector<std::size_t>> group_extents;
	std::vector<std::size_t> group_counts;
	std::vector<std::size_t> extents;
	bool clipped = false;

	for (std::size_t i = 0; i < count; ++i)
	{
		if (!clip_region(regions, i, file_extents, array_extents, extents))
		{
			clipped = true;
			continue;
		}

		if (!std::equal(extents.begin(), extents.end(), whole.begin()))
		{
			clipped = true;
		}

		auto group = find_group(group_extents, extents);
		if (group == group_extents.size())
		{
			group_extents.push_back(extents);
			group_counts.push_back(0);
		}

		group_of_region[i] = group;
		++group_counts[group];
	}

	if (!clipped)
	{
		return false;
	}

	result.reserve(group_extents.size());
	for (std::size_t group = 0; group < group_extents.size(); ++group)
	{
		result.emplace_back(
			make_span(group_extents[group]),
			regions.get_file_rank(),
			regions.get_array_rank()
		);
		result.back().reserve(group_counts[group]);
	}

	for (std::size_t i = 0; i < count; ++i)
	{
		const auto group = group_of_region[i];
		if (group == no_group)
		{
			continue;
		}

		result[group].add(
			regions.get_file_offset(i),
			regions.get_array_offset(i)
		);
	}

	return true;
}

} // namespace em
} // namespace rexlib
