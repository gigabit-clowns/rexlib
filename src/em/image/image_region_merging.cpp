// SPDX-License-Identifier: GPL-3.0-only

#include "image_region_merging.hpp"

#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

namespace rexlib
{
namespace em
{

namespace
{

// The axis a run spans: the last one the extents of the plan do not reach.
// A side that has none of them spans a single position everywhere its
// extents do not, so nothing of it can ever be a run.
bool has_run_axis(const image_transfer_plan &regions) noexcept
{
	return
		regions.get_file_rank() > regions.get_rank() &&
		regions.get_array_rank() > regions.get_rank();
}

bool follows(
	span<const std::size_t> first,
	span<const std::size_t> second,
	std::size_t run_axis
) noexcept
{
	if (second[run_axis] != first[run_axis] + 1)
	{
		return false;
	}

	for (std::size_t axis = 0; axis < first.size(); ++axis)
	{
		if (axis != run_axis && first[axis] != second[axis])
		{
			return false;
		}
	}

	return true;
}

bool are_neighbours(
	const image_transfer_plan &regions,
	std::size_t first,
	std::size_t second
) noexcept
{
	const auto rank = regions.get_rank();

	return
		follows(
			regions.get_file_offset(first),
			regions.get_file_offset(second),
			regions.get_file_rank() - rank - 1
		) &&
		follows(
			regions.get_array_offset(first),
			regions.get_array_offset(second),
			regions.get_array_rank() - rank - 1
		);
}

// Where each run starts and how long it is, in the order they are held.
std::vector<std::pair<std::size_t, std::size_t>> gather_runs(
	const image_transfer_plan &regions
)
{
	std::vector<std::pair<std::size_t, std::size_t>> runs;
	const auto count = regions.get_region_count();

	std::size_t first = 0;
	while (first < count)
	{
		std::size_t last = first + 1;
		while (last < count && are_neighbours(regions, last - 1, last))
		{
			++last;
		}

		runs.emplace_back(first, last - first);
		first = last;
	}

	return runs;
}

std::vector<std::size_t> make_run_extents(
	const image_transfer_plan &regions,
	std::size_t length
)
{
	const auto extents = regions.get_extents();

	std::vector<std::size_t> result;
	result.reserve(extents.size() + 1);
	if (length > 1)
	{
		result.push_back(length);
	}

	result.insert(result.end(), extents.begin(), extents.end());

	return result;
}

std::size_t find_length(
	const std::vector<std::size_t> &lengths,
	std::size_t length
) noexcept
{
	const auto position =
		std::find(lengths.begin(), lengths.end(), length);
	return static_cast<std::size_t>(position - lengths.begin());
}

} // anonymous namespace

bool make_merged_transfer_plans(
	const image_transfer_plan &regions,
	std::vector<image_transfer_plan> &result
)
{
	result.clear();

	if (!has_run_axis(regions))
	{
		return false;
	}

	const auto runs = gather_runs(regions);
	if (runs.size() == regions.get_region_count())
	{
		return false;
	}

	std::vector<std::size_t> lengths;
	for (const auto &run : runs)
	{
		if (find_length(lengths, run.second) == lengths.size())
		{
			lengths.push_back(run.second);
			result.emplace_back(
				make_span(make_run_extents(regions, run.second)),
				regions.get_file_rank(),
				regions.get_array_rank()
			);
		}
	}

	for (const auto &run : runs)
	{
		result[find_length(lengths, run.second)].add(
			regions.get_file_offset(run.first),
			regions.get_array_offset(run.first)
		);
	}

	return true;
}

} // namespace em
} // namespace rexlib
