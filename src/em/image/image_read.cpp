// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_read.hpp>

#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/functional/creation.hpp>

#include <cstddef>
#include <string>
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
	const auto extents = reader.get_extents();
	const auto rank = extents.size();

	auto destination = empty(
		make_contiguous_array_descriptor(extents, reader.get_data_type()),
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
// one region, at `position` along the file's slowest axis and at the
// origin of the array, spanning the trailing core_rank extents.
array read_stack_position(
	const image_reader &reader,
	std::size_t position,
	const execution_context &context
)
{
	const auto file_extents = reader.get_extents();
	const auto core_rank = reader.get_core_rank();
	const span<const std::size_t> core_extents(
		file_extents.data() + (file_extents.size() - core_rank),
		core_rank
	);

	auto destination = empty(
		make_contiguous_array_descriptor(core_extents, reader.get_data_type()),
		memory_resource_affinity::host,
		context
	);

	image_transfer_plan plan(core_extents, file_extents.size(), core_rank);
	std::vector<std::size_t> file_offset(file_extents.size(), 0UL);
	file_offset[0] = position;
	const std::vector<std::size_t> array_offset(core_rank, 0UL);
	plan.add(make_span(file_offset), make_span(array_offset));

	array_ref destination_ref(destination);
	reader.read(destination_ref, plan);

	return destination;
}

} // namespace

array read(
	const std::string &path,
	const image_read_format_manager &manager,
	const execution_context &context
)
{
	return read_whole_file(*manager.open(path), context);
}

array read(
	const image_location &location,
	const image_read_format_manager &manager,
	const execution_context &context
)
{
	const auto reader = manager.open(location.get_path());

	if (!location.has_position())
	{
		return read_whole_file(*reader, context);
	}

	return read_stack_position(
		*reader,
		location.get_position_in_stack(),
		context
	);
}

} // namespace em
} // namespace rexlib
