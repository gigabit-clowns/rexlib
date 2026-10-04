// SPDX-License-Identifier: GPL-3.0-only

#include "image_plan_builders.hpp"

#include <rexlib/core/layout/index_table.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace rexlib
{
namespace em
{

namespace
{

bool get_stack_indexing(span<const image_location> locations)
{
	const auto result =
		!locations.empty() && locations.front().has_index_in_stack();
	for (const auto &location : locations)
	{
		if (location.has_index_in_stack() != result)
		{
			throw std::invalid_argument(
				"make_batch_plan: The batch mixes locations carrying an "
				"index in a stack with locations carrying none."
			);
		}
	}

	return result;
}

// The leading axis of the array counts the slots and the rest are the shape
// of one of them. The file has a leading axis of its own only when it is
// indexed as a stack.
image_transfer_shape make_slot_shape(
	span<const std::size_t> array_extents,
	bool stack_indexing
)
{
	const auto array_rank = array_extents.size();
	const auto slot_rank = array_rank - 1;

	return image_transfer_shape(
		std::vector<std::size_t>(
			array_extents.begin() + 1,
			array_extents.end()
		),
		stack_indexing ? array_rank : slot_rank,
		array_rank
	);
}

// A patch spans from its centre less half its extent, which may begin before
// the image does. The part before it is carried by the array offset instead
// of by a negative file offset, so that the region keeps the extents of a
// whole patch and every patch of the batch shares them.
void place_patch(
	span<const std::size_t> centre,
	const image_transfer_shape &shape,
	std::vector<std::size_t> &file_offset,
	std::vector<std::size_t> &array_offset
) noexcept
{
	const auto patch_extents = shape.get_extents();
	const auto file_leading = shape.get_leading_rank(shape.get_file_rank());
	const auto array_leading = shape.get_leading_rank(shape.get_array_rank());
	for (std::size_t axis = 0; axis < patch_extents.size(); ++axis)
	{
		const auto half =
			static_cast<std::ptrdiff_t>(patch_extents[axis] / 2);
		const auto corner =
			static_cast<std::ptrdiff_t>(centre[axis]) - half;

		file_offset[file_leading + axis] =
			static_cast<std::size_t>(std::max<std::ptrdiff_t>(corner, 0));
		array_offset[array_leading + axis] =
			static_cast<std::size_t>(std::max<std::ptrdiff_t>(-corner, 0));
	}
}

} // anonymous namespace

image_transaction_plan make_batch_plan(
	span<const std::size_t> array_extents,
	span<const image_location> locations
)
{
	if (array_extents.empty())
	{
		throw std::invalid_argument(
			"make_batch_plan: The array has no extents, where its leading "
			"one is the batch size and the rest are the shape of one element."
		);
	}

	if (array_extents.front() != locations.size())
	{
		throw std::invalid_argument(
			"make_batch_plan: The leading extent of the array is not the "
			"number of locations."
		);
	}

	const auto stack_indexing = get_stack_indexing(locations);
	image_transaction_plan transaction(
		make_slot_shape(array_extents, stack_indexing)
	);
	transaction.reserve(locations.size(), locations.size());

	const auto &shape = transaction.get_shape();
	std::vector<std::size_t> file_offset(shape.get_file_rank(), 0UL);
	std::vector<std::size_t> array_offset(shape.get_array_rank(), 0UL);
	for (std::size_t i = 0; i < locations.size(); ++i)
	{
		const auto &location = locations[i];

		array_offset[0] = i;
		if (stack_indexing)
		{
			file_offset[0] = location.get_index_in_stack();
		}

		transaction.add(
			transaction.add_file(location.get_path()),
			make_span(file_offset),
			make_span(array_offset)
		);
	}

	return transaction;
}

image_transaction_plan make_patch_plan(
	span<const std::size_t> array_extents,
	const image_location &location,
	const index_table &centres
)
{
	if (array_extents.empty())
	{
		throw std::invalid_argument(
			"make_patch_plan: The array has no extents, where its leading "
			"one is the batch size and the rest are the shape of one patch."
		);
	}

	const auto batch_size = centres.get_index_count();
	if (array_extents.front() != batch_size)
	{
		throw std::invalid_argument(
			"make_patch_plan: The leading extent of the array is not the "
			"number of centres."
		);
	}

	if (centres.get_rank() != array_extents.size() - 1)
	{
		throw std::invalid_argument(
			"make_patch_plan: The centres do not have the rank of one patch, "
			"which is one less than that of the array."
		);
	}

	const auto stack_indexing = location.has_index_in_stack();
	image_transaction_plan transaction(
		make_slot_shape(array_extents, stack_indexing)
	);
	transaction.reserve(1, batch_size);

	const auto &shape = transaction.get_shape();
	std::vector<std::size_t> file_offset(shape.get_file_rank(), 0UL);
	std::vector<std::size_t> array_offset(shape.get_array_rank(), 0UL);
	if (stack_indexing)
	{
		file_offset[0] = location.get_index_in_stack();
	}

	const auto file_index = transaction.add_file(location.get_path());
	for (std::size_t i = 0; i < batch_size; ++i)
	{
		array_offset[0] = i;
		place_patch(centres.get(i), shape, file_offset, array_offset);

		transaction.add(
			file_index,
			make_span(file_offset),
			make_span(array_offset)
		);
	}

	return transaction;
}

image_transfer_plan make_location_plan(
	const image_descriptor &file,
	const image_location &location
)
{
	const auto file_extents = file.get_extents();
	const auto extents = location.has_index_in_stack()
		? get_core_extents(file)
		: file_extents;

	image_transfer_plan plan(
		image_transfer_shape(
			std::vector<std::size_t>(extents.begin(), extents.end()),
			file_extents.size(),
			extents.size()
		)
	);

	std::vector<std::size_t> file_offset(file_extents.size(), 0UL);
	if (location.has_index_in_stack())
	{
		file_offset[0] = location.get_index_in_stack();
	}

	const std::vector<std::size_t> array_offset(extents.size(), 0UL);
	plan.add(make_span(file_offset), make_span(array_offset));

	return plan;
}

} // namespace em
} // namespace rexlib
