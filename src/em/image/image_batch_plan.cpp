// SPDX-License-Identifier: GPL-3.0-only

#include "image_batch_plan.hpp"

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

[[noreturn]]
void throw_invalid_argument(const char *context, const char *what)
{
	throw std::invalid_argument(std::string(context) + ": " + what);
}

bool has_position(const image_location &location) noexcept
{
	return location.get_position_in_stack() != image_location::no_position;
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
		!locations.empty() && has_position(locations.front());
	const auto array_rank = array_extents.size();
	const auto core_rank = array_rank - 1;
	const auto file_rank = stack_indexing ? array_rank : core_rank;
	const span<const std::size_t> core_extents(
		array_extents.data() + 1,
		core_rank
	);

	image_transaction_plan transaction(core_extents, file_rank, array_rank);
	transaction.reserve(batch_size, batch_size);

	std::vector<std::size_t> file_offset(file_rank, 0UL);
	std::vector<std::size_t> array_offset(array_rank, 0UL);
	for (std::size_t i = 0; i < batch_size; ++i)
	{
		const auto &location = locations[i];
		if (has_position(location) != stack_indexing)
		{
			throw_invalid_argument(
				context,
				"The batch mixes locations carrying a position in a stack "
				"with locations carrying none."
			);
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
