// SPDX-License-Identifier: GPL-3.0-only

#include "image_batch_plan.hpp"

#include <rexlib/core/platform/attributes.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>

#include <stdexcept>
#include <string>
#include <vector>

namespace rexlib
{
namespace em
{

namespace
{

REXLIB_NORETURN
void throw_invalid_argument(const char *context, const char *what)
{
	throw std::invalid_argument(std::string(context) + ": " + what);
}

// Two slots belong to one region when they are neighbours on both sides:
// consecutive positions of one file, and consecutive slots of the array,
// which slots next to each other in a batch always are.
bool are_adjacent(
	const image_location &first,
	const image_location &second
) noexcept
{
	return
		first.has_position() &&
		second.has_position() &&
		second.get_position_in_stack() ==
			first.get_position_in_stack() + 1 &&
		second.get_path() == first.get_path();
}

// Whether the batch is one run of neighbours from end to end, which is what
// a stack read or written a batch at a time is. A plan carries one set of
// extents for every region it holds, so a batch that is only partly made of
// neighbours cannot merge the part that is and stays one region per slot.
bool is_one_run(span<const image_location> locations) noexcept
{
	if (locations.size() < 2)
	{
		return false;
	}

	for (std::size_t i = 1; i < locations.size(); ++i)
	{
		if (!are_adjacent(locations[i - 1], locations[i]))
		{
			return false;
		}
	}

	return true;
}

std::vector<std::size_t> make_region_extents(
	span<const std::size_t> array_extents,
	std::size_t slots_per_region
)
{
	std::vector<std::size_t> extents;
	extents.reserve(array_extents.size());
	if (slots_per_region > 1)
	{
		extents.push_back(slots_per_region);
	}

	extents.insert(
		extents.end(),
		array_extents.begin() + 1,
		array_extents.end()
	);

	return extents;
}

} // anonymous namespace

image_transaction_plan make_batch_transaction_plan(
	span<const std::size_t> array_extents,
	span<const image_location> locations,
	const char *context
)
{
	if (array_extents.empty())
	{
		throw_invalid_argument(
			context,
			"The array has no extents, where its leading one is the batch "
			"size and the rest are the shape of one element."
		);
	}

	const auto batch_size = locations.size();
	if (array_extents.front() != batch_size)
	{
		throw_invalid_argument(
			context,
			"The leading extent of the array is not the number of locations."
		);
	}

	const auto stack_indexing =
		!locations.empty() && locations.front().has_position();
	const auto array_rank = array_extents.size();
	const auto core_rank = array_rank - 1;
	const auto file_rank = stack_indexing ? array_rank : core_rank;
	const auto slots_per_region = is_one_run(locations) ? batch_size : 1;
	const auto extents = make_region_extents(array_extents, slots_per_region);

	image_transaction_plan transaction(
		make_span(extents),
		file_rank,
		array_rank
	);

	const auto region_count = batch_size / slots_per_region;
	transaction.reserve(region_count, region_count);

	std::vector<std::size_t> file_offset(file_rank, 0UL);
	std::vector<std::size_t> array_offset(array_rank, 0UL);
	for (std::size_t i = 0; i < batch_size; ++i)
	{
		const auto &location = locations[i];
		if (location.has_position() != stack_indexing)
		{
			throw_invalid_argument(
				context,
				"The batch mixes locations carrying a position in a stack "
				"with locations carrying none."
			);
		}

		if (i % slots_per_region != 0)
		{
			continue;
		}

		array_offset[0] = i;
		if (stack_indexing)
		{
			file_offset[0] = location.get_position_in_stack();
		}

		transaction.add(
			transaction.add_file(location.get_path()),
			make_span(file_offset),
			make_span(array_offset)
		);
	}

	return transaction;
}

} // namespace em
} // namespace rexlib
