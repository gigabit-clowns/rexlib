// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/buffer_image_scratch.hpp>

#include "buffer_image_scratch_entry.hpp"

#include <rexlib/core/exceptions/unsupported_capability_error.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/mapped_file_buffer.hpp>
#include <rexlib/core/hardware/memory_allocator.hpp>
#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/memory/align.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_location_grouping.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_reader_provider.hpp>

#include <algorithm>
#include <functional>
#include <numeric>
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
	const image_location_grouping::group &group,
	const image_descriptor &file
)
{
	const auto index_count = file.get_extents().front();
	const auto named = group.get_indices();
	if (!named.empty() && named.back() >= index_count)
	{
		throw std::out_of_range(
			group.get_path() + ": buffer_image_scratch: A location has a "
			"stack index that the file does not have."
		);
	}

	if (!group.is_whole())
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

void check_storage(const buffer *storage)
{
	if (storage == nullptr)
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
}

void check_run_length(std::size_t run_length)
{
	if (run_length == 0)
	{
		throw std::invalid_argument(
			"buffer_image_scratch: A run must span at least one index."
		);
	}
}

void check_alignment(
	const buffer &storage,
	const image_descriptor &file,
	const std::string &path
)
{
	if (!is_aligned(storage.get_host_ptr(), get_size(file.get_data_type())))
	{
		throw std::invalid_argument(
			path + ": buffer_image_scratch: The buffer is not aligned for "
			"the data type of the file."
		);
	}
}

void check_size(std::size_t size)
{
	if (size == 0)
	{
		throw std::invalid_argument(
			"buffer_image_scratch: There is nothing to hold. The locations "
			"name no image, or the maximum size has no room for one."
		);
	}
}

// Places the values of the entries of a scratch one after another in a
// number of bytes, each aligned for its data type.
class storage_cursor
{
public:
	explicit storage_cursor(std::size_t capacity) noexcept
		: m_capacity(capacity)
		, m_used(0)
	{
	}

	std::size_t get_used() const noexcept
	{
		return m_used;
	}

	std::size_t count_fitting(const image_descriptor &file) const noexcept
	{
		const auto first = get_first_byte(file);
		if (first >= m_capacity)
		{
			return 0;
		}

		return (m_capacity - first) / compute_slot_size(file);
	}

	// Takes the room for a number of indices of a file, and gives the
	// element at which they start.
	std::size_t place(
		const image_descriptor &file,
		std::size_t index_count
	) noexcept
	{
		const auto first = get_first_byte(file);
		m_used = first + index_count * compute_slot_size(file);

		return first / get_size(file.get_data_type());
	}

private:
	std::size_t get_first_byte(const image_descriptor &file) const noexcept
	{
		return align_ceil(m_used, get_size(file.get_data_type()));
	}

	std::size_t m_capacity;
	std::size_t m_used;
};

// The size of the storage that a scratch of some locations uses, when it
// may use no more than a maximum. It is a dry run of the constructor: the
// files are taken and placed the same way, and nothing is stored.
std::size_t compute_size(
	const image_location_grouping &locations,
	image_reader_provider &files,
	std::size_t max_size
)
{
	const auto group_count = locations.get_group_count();

	storage_cursor cursor(max_size);
	for (std::size_t index = 0; index < group_count; ++index)
	{
		const auto group = locations.get_group(index);
		const auto file = files.acquire(group.get_path());
		REXLIB_ASSERT(file);

		const auto &descriptor = file->get_descriptor();
		const auto named_count = get_named_indices(group, descriptor).size();
		if (named_count == 0 || compute_slot_size(descriptor) == 0)
		{
			continue;
		}

		const auto held_count =
			std::min(named_count, cursor.count_fitting(descriptor));
		if (held_count > 0)
		{
			cursor.place(descriptor, held_count);
		}

		if (held_count < named_count)
		{
			break;
		}
	}

	return cursor.get_used();
}

} // anonymous namespace

buffer_image_scratch::buffer_image_scratch(
	const image_location_grouping &locations,
	image_reader_provider &files,
	std::shared_ptr<buffer> storage,
	std::size_t run_length
)
{
	check_storage(storage.get());
	check_run_length(run_length);

	const auto group_count = locations.get_group_count();

	storage_cursor cursor(storage->get_size());
	for (std::size_t index = 0; index < group_count; ++index)
	{
		const auto group = locations.get_group(index);
		const auto &path = group.get_path();
		const auto file = files.acquire(path);
		REXLIB_ASSERT(file);

		const auto &descriptor = file->get_descriptor();
		auto indices = get_named_indices(group, descriptor);
		if (indices.empty() || compute_slot_size(descriptor) == 0)
		{
			continue;
		}

		check_alignment(*storage, descriptor, path);

		const auto named_count = indices.size();
		const auto held_count =
			std::min(named_count, cursor.count_fitting(descriptor));
		if (held_count > 0)
		{
			indices.resize(held_count);
			const auto first_element = cursor.place(descriptor, held_count);
			m_entries.emplace(
				path,
				std::make_shared<buffer_image_scratch_entry>(
					std::move(indices),
					array(
						storage,
						make_values_descriptor(
							descriptor,
							held_count,
							first_element
						)
					),
					run_length
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

std::shared_ptr<image_scratch> create_host_image_scratch(
	const image_location_grouping &locations,
	image_reader_provider &files,
	std::size_t run_length,
	std::size_t max_size
)
{
	check_run_length(run_length);

	const auto size = compute_size(locations, files, max_size);
	check_size(size);

	const auto allocator = get_host_memory_resource().create_allocator();

	return std::make_shared<buffer_image_scratch>(
		locations,
		files,
		allocator->allocate(size, allocator->get_max_alignment()),
		run_length
	);
}

std::shared_ptr<image_scratch> create_mapped_file_image_scratch(
	const image_location_grouping &locations,
	image_reader_provider &files,
	const std::string &path,
	std::size_t run_length,
	std::size_t max_size
)
{
	check_run_length(run_length);

	const auto size = compute_size(locations, files, max_size);
	check_size(size);

	return std::make_shared<buffer_image_scratch>(
		locations,
		files,
		create_mapped_file_buffer(path, size),
		run_length
	);
}

} // namespace em
} // namespace rexlib
