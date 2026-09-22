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
// neighbours cannot merge the part that is.
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

bool get_stack_indexing(
	span<const image_location> locations,
	const char *context
)
{
	const auto result = !locations.empty() && locations.front().has_position();
	for (const auto &location : locations)
	{
		if (location.has_position() != result)
		{
			throw_invalid_argument(
				context,
				"The batch mixes locations carrying a position in a stack "
				"with locations carrying none."
			);
		}
	}

	return result;
}

// The whole batch as one region: its extents are the array's, since the
// slots it spans are every slot there is.
image_transaction_plan make_run_plan(
	span<const std::size_t> array_extents,
	const image_location &first
)
{
	const auto rank = array_extents.size();

	image_transaction_plan transaction(array_extents, rank, rank);
	transaction.reserve(1, 1);

	std::vector<std::size_t> file_offset(rank, 0UL);
	file_offset[0] = first.get_position_in_stack();
	const std::vector<std::size_t> array_offset(rank, 0UL);

	transaction.add(
		transaction.add_file(first.get_path()),
		make_span(file_offset),
		make_span(array_offset)
	);

	return transaction;
}

// One region per slot, each the shape of one element.
image_transaction_plan make_slot_plan(
	span<const std::size_t> array_extents,
	span<const image_location> locations,
	bool stack_indexing
)
{
	const auto array_rank = array_extents.size();
	const auto core_rank = array_rank - 1;
	const auto file_rank = stack_indexing ? array_rank : core_rank;
	const span<const std::size_t> core_extents(
		array_extents.data() + 1,
		core_rank
	);

	image_transaction_plan transaction(core_extents, file_rank, array_rank);
	transaction.reserve(locations.size(), locations.size());

	std::vector<std::size_t> file_offset(file_rank, 0UL);
	std::vector<std::size_t> array_offset(array_rank, 0UL);
	for (std::size_t i = 0; i < locations.size(); ++i)
	{
		const auto &location = locations[i];

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

} // anonymous namespace

image_transaction_plan make_batch_plan(
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

	if (array_extents.front() != locations.size())
	{
		throw_invalid_argument(
			context,
			"The leading extent of the array is not the number of locations."
		);
	}

	const auto stack_indexing = get_stack_indexing(locations, context);

	return is_one_run(locations)
		? make_run_plan(array_extents, locations.front())
		: make_slot_plan(array_extents, locations, stack_indexing);
}

} // namespace em
} // namespace rexlib
