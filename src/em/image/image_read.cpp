// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_read.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/counting_completion.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_reader_provider.hpp>
#include <rexlib/em/image/image_source.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/index_table.hpp>
#include <rexlib/functional/creation.hpp>

#include <em/image/image_batch_plan.hpp>

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace rexlib
{
namespace em
{

namespace
{

// Reads a file into an array covering all of its extents: one region, at
// the origin of both sides, spanning everything.
array read_whole_file(
	const image_reader &reader,
	const execution_context &context
)
{
	const auto &descriptor = reader.get_descriptor();
	const auto extents = descriptor.get_extents();
	const auto rank = extents.size();

	auto destination = empty(
		make_contiguous_array_descriptor(extents, descriptor.get_data_type()),
		memory_resource_affinity::host,
		context
	);

	image_transfer_plan plan(extents, rank, rank);
	const std::vector<std::size_t> origin(rank, 0UL);
	plan.add(make_span(origin), make_span(origin));

	array_ref destination_ref(destination);
	reader.read(destination_ref, plan);

	return destination;
}

// Reads one element of a stack into an array covering just its core shape:
// one region, at `index_in_stack` along the file's slowest axis and at the
// origin of the array, spanning the trailing core_rank extents.
array read_stack_slice(
	const image_reader &reader,
	std::size_t index_in_stack,
	const execution_context &context
)
{
	const auto &descriptor = reader.get_descriptor();
	const auto file_extents = descriptor.get_extents();
	const auto core_extents = get_core_extents(descriptor);
	const auto core_rank = core_extents.size();

	auto destination = empty(
		make_contiguous_array_descriptor(
			core_extents,
			descriptor.get_data_type()
		),
		memory_resource_affinity::host,
		context
	);

	image_transfer_plan plan(core_extents, file_extents.size(), core_rank);
	std::vector<std::size_t> file_offset(file_extents.size(), 0UL);
	file_offset[0] = index_in_stack;
	const std::vector<std::size_t> array_offset(core_rank, 0UL);
	plan.add(make_span(file_offset), make_span(array_offset));

	array_ref destination_ref(destination);
	reader.read(destination_ref, plan);

	return destination;
}

// A patch spans from its centre less half its extent, which may begin before
// the image does. The part before it is carried by the array offset instead
// of by a negative file offset, so that the region the source is handed keeps
// the extents of a whole patch and every patch of the batch shares them.
void place_patch(
	span<const std::size_t> centre,
	span<const std::size_t> patch_extents,
	std::size_t file_leading,
	std::vector<std::size_t> &file_offset,
	std::vector<std::size_t> &array_offset
) noexcept
{
	for (std::size_t axis = 0; axis < patch_extents.size(); ++axis)
	{
		const auto half =
			static_cast<std::ptrdiff_t>(patch_extents[axis] / 2);
		const auto corner =
			static_cast<std::ptrdiff_t>(centre[axis]) - half;

		file_offset[file_leading + axis] =
			static_cast<std::size_t>(std::max<std::ptrdiff_t>(corner, 0));
		array_offset[1 + axis] =
			static_cast<std::size_t>(std::max<std::ptrdiff_t>(-corner, 0));
	}
}

} // anonymous namespace

array read(
	const std::string &path,
	image_reader_provider &readers,
	const execution_context &context
)
{
	return read_whole_file(*readers.acquire(path), context);
}

array read(
	const image_location &location,
	image_reader_provider &readers,
	const execution_context &context
)
{
	const auto reader = readers.acquire(location.get_path());

	if (!location.has_index_in_stack())
	{
		return read_whole_file(*reader, context);
	}

	return read_stack_slice(*reader, location.get_index_in_stack(), context);
}

std::shared_ptr<completion> read_batch_async(
	const image_source &source,
	array destination,
	span<const image_location> locations
)
{
	std::vector<std::size_t> array_extents;
	destination.get_descriptor().get_layout().get_extents(array_extents);

	const auto transaction = make_batch_plan(
		make_span(array_extents),
		locations,
		"read_batch_async"
	);

	return source.read(std::move(destination), transaction);
}

std::shared_ptr<completion> read_patches_async(
	const image_source &source,
	array destination,
	const image_location &location,
	const index_table &centres
)
{
	std::vector<std::size_t> array_extents;
	destination.get_descriptor().get_layout().get_extents(array_extents);

	if (array_extents.empty())
	{
		throw std::invalid_argument(
			"read_patches_async: The destination has no extents, where its "
			"leading one is the batch size and the rest are the shape of one "
			"patch."
		);
	}

	const auto batch_size = centres.get_index_count();
	if (array_extents.front() != batch_size)
	{
		throw std::invalid_argument(
			"read_patches_async: The leading extent of the destination is not "
			"the number of centres."
		);
	}

	const auto array_rank = array_extents.size();
	const auto patch_rank = array_rank - 1;
	if (centres.get_rank() != patch_rank)
	{
		throw std::invalid_argument(
			"read_patches_async: The centres do not have the rank of one "
			"patch, which is one less than that of the destination."
		);
	}

	if (batch_size == 0)
	{
		return std::make_shared<counting_completion>(0);
	}

	const auto stack_indexing = location.has_index_in_stack();
	const auto file_rank = stack_indexing ? array_rank : patch_rank;
	const auto file_leading = file_rank - patch_rank;
	const span<const std::size_t> patch_extents(
		array_extents.data() + 1,
		patch_rank
	);

	image_transaction_plan transaction(patch_extents, file_rank, array_rank);
	transaction.reserve(1, batch_size);

	const auto file_index = transaction.add_file(location.get_path());
	std::vector<std::size_t> file_offset(file_rank, 0UL);
	std::vector<std::size_t> array_offset(array_rank, 0UL);
	if (stack_indexing)
	{
		file_offset[0] = location.get_index_in_stack();
	}

	for (std::size_t i = 0; i < batch_size; ++i)
	{
		array_offset[0] = i;
		place_patch(
			centres.get(i),
			patch_extents,
			file_leading,
			file_offset,
			array_offset
		);

		transaction.add(
			file_index,
			make_span(file_offset),
			make_span(array_offset)
		);
	}

	return source.read(std::move(destination), transaction);
}

} // namespace em
} // namespace rexlib
