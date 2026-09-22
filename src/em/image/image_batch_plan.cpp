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

	return make_slot_plan(
		array_extents,
		locations,
		get_stack_indexing(locations, context)
	);
}

} // namespace em
} // namespace rexlib
