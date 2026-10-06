// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/buffer_image_scratch.hpp>

#include "buffer_image_scratch_entry.hpp"
#include "image_location_grouping.hpp"
#include "image_scratch_slots.hpp"

#include <rexlib/core/hardware/memory_allocator.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_reader_provider.hpp>

#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

namespace rexlib
{
namespace em
{

namespace
{

array_descriptor make_values_descriptor(
	const image_descriptor &file,
	std::size_t index_count
)
{
	const auto file_extents = file.get_extents();
	std::vector<std::size_t> extents(file_extents.begin(), file_extents.end());
	extents.front() = index_count;

	return make_contiguous_array_descriptor(
		make_span(extents),
		file.get_data_type()
	);
}

std::vector<std::size_t> get_named_indices(
	const image_location_grouping &grouping,
	std::size_t file_index,
	const image_descriptor &file
)
{
	const auto index_count = file.get_extents().front();
	const auto named = grouping.get_indices(file_index);
	if (!named.empty() && named.back() >= index_count)
	{
		throw std::out_of_range(
			grouping.get_path(file_index) + ": buffer_image_scratch: A "
			"location has a stack index that the file does not have."
		);
	}

	if (!grouping.is_whole(file_index))
	{
		return std::vector<std::size_t>(named.begin(), named.end());
	}

	std::vector<std::size_t> indices(index_count);
	std::iota(indices.begin(), indices.end(), std::size_t(0));

	return indices;
}

std::shared_ptr<image_scratch_entry> make_entry(
	const image_descriptor &file,
	std::vector<std::size_t> indices,
	memory_allocator &allocator,
	std::size_t run_length
)
{
	auto descriptor = make_values_descriptor(file, indices.size());
	auto storage = allocator.allocate(
		compute_storage_requirement(descriptor),
		std::min(allocator.get_max_alignment(), get_size(file.get_data_type()))
	);

	return std::make_shared<buffer_image_scratch_entry>(
		image_scratch_slots(std::move(indices)),
		array(std::move(storage), std::move(descriptor)),
		run_length
	);
}

} // anonymous namespace

buffer_image_scratch::buffer_image_scratch(
	span<const image_location> locations,
	image_reader_provider &files,
	memory_allocator &allocator,
	std::size_t capacity,
	std::size_t run_size
)
{
	const image_location_grouping grouping(locations);
	const auto file_count = grouping.get_file_count();

	auto remaining = capacity;
	for (std::size_t file_index = 0; file_index < file_count; ++file_index)
	{
		if (remaining == 0)
		{
			break;
		}

		const auto &path = grouping.get_path(file_index);
		const auto file = files.acquire(path);
		REXLIB_ASSERT(file);

		const auto &descriptor = file->get_descriptor();
		auto indices = get_named_indices(grouping, file_index, descriptor);
		const auto slot_size =
			compute_storage_requirement(make_values_descriptor(descriptor, 1));
		if (indices.empty() || slot_size == 0)
		{
			continue;
		}

		const auto named_count = indices.size();
		const auto held_count = std::min(named_count, remaining / slot_size);
		if (held_count > 0)
		{
			indices.resize(held_count);
			remaining -= held_count * slot_size;
			m_entries.emplace(
				path,
				make_entry(
					descriptor,
					std::move(indices),
					allocator,
					std::max<std::size_t>(run_size / slot_size, 1)
				)
			);
		}

		if (held_count < named_count)
		{
			break;
		}
	}
}

buffer_image_scratch::~buffer_image_scratch() = default;

std::shared_ptr<image_scratch_entry>
buffer_image_scratch::find(const std::string &path)
{
	const auto ite = m_entries.find(path);
	if (ite == m_entries.end())
	{
		return nullptr;
	}

	return ite->second;
}

std::shared_ptr<const image_scratch_entry>
buffer_image_scratch::find(const std::string &path) const
{
	const auto ite = m_entries.find(path);
	if (ite == m_entries.end())
	{
		return nullptr;
	}

	return ite->second;
}

} // namespace em
} // namespace rexlib
