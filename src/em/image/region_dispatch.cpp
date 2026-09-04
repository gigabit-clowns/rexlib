// SPDX-License-Identifier: GPL-3.0-only

#include <em/image/region_dispatch.hpp>

#include <rexlib/em/image/image_transaction_plan.hpp>
#include <rexlib/em/image/region_grouping.hpp>

namespace rexlib
{
namespace em
{

std::size_t count_files_with_regions(const region_grouping &grouping)
{
	std::size_t count = 0;
	for (
		std::size_t file_index = 0;
		file_index < grouping.get_file_count();
		++file_index
	)
	{
		if (grouping.get_region_count(file_index) > 0)
		{
			++count;
		}
	}

	return count;
}

image_transfer_plan build_file_transfer_plan(
	const image_transaction_plan &plan,
	const region_grouping &grouping,
	std::size_t file_index
)
{
	const auto first = grouping.get_first_position(file_index);
	const auto count = grouping.get_region_count(file_index);

	image_transfer_plan transfer(
		plan.get_extents(),
		plan.get_file_rank(),
		plan.get_array_rank()
	);
	transfer.reserve(count);
	for (auto i = first; i < first + count; ++i)
	{
		const auto region = grouping.get_region(i);
		transfer.add(
			plan.get_file_offset(region),
			plan.get_array_offset(region)
		);
	}

	return transfer;
}

} // namespace em
} // namespace rexlib
