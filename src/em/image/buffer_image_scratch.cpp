// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/buffer_image_scratch.hpp>

#include "buffer_image_scratch_entry.hpp"
#include "image_location_grouping.hpp"
#include "image_scratch_slots.hpp"

#include <rexlib/core/exceptions/unsupported_capability_error.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/memory/align.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_reader_provider.hpp>

#include <algorithm>
#include <functional>
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

std::size_t compute_slot_size(const image_descriptor &file) noexcept
{
	const auto extents = file.get_extents();
	return std::accumulate(
		extents.begin() + 1,
		extents.end(),
		get_size(file.get_data_type()),
		std::multiplies<std::size_t>()
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

array_descriptor make_values_descriptor(
	const image_descriptor &file,
	std::size_t index_count,
	std::size_t first_element
)
{
	const auto file_extents = file.get_extents();
	std::vector<std::size_t> extents(file_extents.begin(), file_extents.end());
	extents.front() = index_count;

	std::vector<std::ptrdiff_t> strides(extents.size());
	std::ptrdiff_t stride = 1;
	for (auto axis = extents.size(); axis > 0; --axis)
	{
		strides[axis - 1] = stride;
		stride *= static_cast<std::ptrdiff_t>(extents[axis - 1]);
	}

	return array_descriptor(
		strided_layout::make_custom_layout(
			make_span(extents),
			make_span(strides),
			static_cast<std::ptrdiff_t>(first_element)
		),
		file.get_data_type()
	);
}

} // anonymous namespace

buffer_image_scratch::buffer_image_scratch(
	span<const image_location> locations,
	image_reader_provider &files,
	std::shared_ptr<buffer> storage,
	std::size_t run_size
)
{
	if (!storage)
	{
		throw std::invalid_argument(
			"buffer_image_scratch: The buffer must not be null."
		);
	}

	if (storage->get_host_ptr() == nullptr)
	{
		throw unsupported_capability_error(
			"buffer_image_scratch: The buffer can not be reached from the "
			"host."
		);
	}

	const image_location_grouping grouping(locations);
	const auto file_count = grouping.get_file_count();
	const auto capacity = storage->get_size();

	std::size_t used = 0;
	for (std::size_t file_index = 0; file_index < file_count; ++file_index)
	{
		if (used >= capacity)
		{
			break;
		}

		const auto &path = grouping.get_path(file_index);
		const auto file = files.acquire(path);
		REXLIB_ASSERT(file);

		const auto &descriptor = file->get_descriptor();
		auto indices = get_named_indices(grouping, file_index, descriptor);
		const auto slot_size = compute_slot_size(descriptor);
		if (indices.empty() || slot_size == 0)
		{
			continue;
		}

		const auto element_size = get_size(descriptor.get_data_type());
		if (!is_aligned(storage->get_host_ptr(), element_size))
		{
			throw std::invalid_argument(
				path + ": buffer_image_scratch: The buffer is not aligned "
				"for the data type of the file."
			);
		}

		const auto first = align_ceil(used, element_size);
		const auto room = first < capacity ? capacity - first : 0;
		const auto named_count = indices.size();
		const auto held_count = std::min(named_count, room / slot_size);
		if (held_count > 0)
		{
			indices.resize(held_count);
			used = first + held_count * slot_size;
			m_entries.emplace(
				path,
				std::make_shared<buffer_image_scratch_entry>(
					image_scratch_slots(std::move(indices)),
					array(
						storage,
						make_values_descriptor(
							descriptor,
							held_count,
							first / element_size
						)
					),
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
