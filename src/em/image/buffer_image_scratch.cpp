// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/buffer_image_scratch.hpp>

#include "buffer_image_scratch_entry.hpp"

#include <rexlib/core/exceptions/file_error.hpp>
#include <rexlib/core/exceptions/unsupported_capability_error.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/mapped_file_buffer.hpp>
#include <rexlib/core/hardware/memory_allocator.hpp>
#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/memory/align.hpp>
#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_location_grouping.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_reader_provider.hpp>

#include <boost/filesystem/operations.hpp>

#include <algorithm>
#include <cstdint>
#include <cstring>
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

array_descriptor
make_flags_descriptor(std::size_t flag_count, std::size_t first_flag)
{
	const std::size_t extents[1] = {flag_count};
	const std::ptrdiff_t strides[1] = {1};

	return array_descriptor(
		strided_layout::make_custom_layout(
			make_span(extents, 1),
			make_span(strides, 1),
			static_cast<std::ptrdiff_t>(first_flag)
		),
		numerical_type::uint64
	);
}

std::size_t
compute_run_count(std::size_t index_count, std::size_t run_length) noexcept
{
	return index_count == 0 ? 0 : (index_count - 1) / run_length + 1;
}

// When a file was last written, or zero if that can not be told.
std::uint64_t get_modification_time(const std::string &path) noexcept
{
	boost::system::error_code error;
	const auto time = boost::filesystem::last_write_time(path, error);
	if (error || time <= 0)
	{
		return 0;
	}

	return static_cast<std::uint64_t>(time);
}

// The fingerprint is an FNV-1a hash, since what the standard library and
// Boost hash to may change between their versions.
const std::uint64_t fingerprint_seed = 14695981039346656037ULL;
const std::uint64_t fingerprint_prime = 1099511628211ULL;

// To be changed whenever a scratch is laid out differently in its storage.
const std::uint64_t format_version = 1;

std::uint64_t
hash_bytes(std::uint64_t hash, const byte *bytes, std::size_t size) noexcept
{
	for (std::size_t position = 0; position < size; ++position)
	{
		hash ^= as_uint8(bytes[position]);
		hash *= fingerprint_prime;
	}

	return hash;
}

std::uint64_t hash_value(std::uint64_t hash, std::uint64_t value) noexcept
{
	return hash_bytes(hash, as_bytes(&value), sizeof(value));
}

std::uint64_t read_fingerprint(const buffer &storage) noexcept
{
	std::uint64_t fingerprint = 0;
	std::memcpy(&fingerprint, storage.get_host_ptr(), sizeof(fingerprint));

	return fingerprint;
}

void write_fingerprint(buffer &storage, std::uint64_t fingerprint) noexcept
{
	std::memcpy(storage.get_host_ptr(), &fingerprint, sizeof(fingerprint));
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

	if (!is_aligned(storage->get_host_ptr(), alignof(std::uint64_t)))
	{
		throw std::invalid_argument(
			"buffer_image_scratch: The buffer is not aligned for 64-bit "
			"integers."
		);
	}

	if (storage->get_size() < sizeof(std::uint64_t))
	{
		throw std::invalid_argument(
			"buffer_image_scratch: The buffer has no room for a scratch."
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

// Lays out a scratch in a number of bytes: its fingerprint, then for each
// entry its flags followed by its values, each aligned for its type. It also
// computes the fingerprint of what it lays out.
class storage_cursor
{
public:
	storage_cursor(std::size_t capacity, std::size_t run_length) noexcept
		: m_capacity(capacity)
		, m_run_length(run_length)
		, m_used(sizeof(std::uint64_t))
		, m_entry_count(0)
		, m_fingerprint(
			hash_value(
				hash_value(fingerprint_seed, format_version),
				run_length
			)
		)
	{
	}

	std::size_t get_used() const noexcept
	{
		return m_used;
	}

	std::size_t get_entry_count() const noexcept
	{
		return m_entry_count;
	}

	std::uint64_t get_fingerprint() const noexcept
	{
		return m_fingerprint;
	}

	// Places an entry for as many of some indices of a file as fit, and
	// gives how many do. An entry of no index is not placed.
	std::size_t place(
		const std::string &path,
		const image_descriptor &file,
		span<const std::size_t> indices
	)
	{
		const auto index_count = count_fitting(file, indices.size());
		if (index_count == 0)
		{
			return 0;
		}

		const auto flag_count = compute_run_count(index_count, m_run_length);
		const auto first_flag = get_first_flag_byte();
		const auto first_value = get_first_value_byte(file, flag_count);
		m_flags = make_flags_descriptor(
			flag_count,
			first_flag / sizeof(std::uint64_t)
		);
		m_values = make_values_descriptor(
			file,
			index_count,
			first_value / get_size(file.get_data_type())
		);

		m_used = first_value + index_count * compute_slot_size(file);
		++m_entry_count;
		add_to_fingerprint(
			path,
			file,
			make_span(indices.data(), index_count)
		);

		return index_count;
	}

	// Where the entry placed last has its flags.
	const array_descriptor& get_flags() const noexcept
	{
		return m_flags;
	}

	// Where the entry placed last has its values.
	const array_descriptor& get_values() const noexcept
	{
		return m_values;
	}

private:
	std::size_t get_first_flag_byte() const noexcept
	{
		return align_ceil(m_used, alignof(std::uint64_t));
	}

	std::size_t get_first_value_byte(
		const image_descriptor &file,
		std::size_t flag_count
	) const noexcept
	{
		return align_ceil(
			get_first_flag_byte() + flag_count * sizeof(std::uint64_t),
			get_size(file.get_data_type())
		);
	}

	bool fits(
		const image_descriptor &file,
		std::size_t index_count
	) const noexcept
	{
		const auto flag_count = compute_run_count(index_count, m_run_length);
		const auto first_value = get_first_value_byte(file, flag_count);
		if (first_value > m_capacity)
		{
			return false;
		}

		const auto room = m_capacity - first_value;
		return index_count <= room / compute_slot_size(file);
	}

	// The flags take room too, and how many there are depends on how many
	// indices are placed, so the count is searched for.
	std::size_t count_fitting(
		const image_descriptor &file,
		std::size_t index_count
	) const noexcept
	{
		if (fits(file, index_count))
		{
			return index_count;
		}

		std::size_t fitting = 0;
		auto excluded = index_count;
		while (excluded - fitting > 1)
		{
			const auto middle = fitting + (excluded - fitting) / 2;
			if (fits(file, middle))
			{
				fitting = middle;
			}
			else
			{
				excluded = middle;
			}
		}

		return fitting;
	}

	void add_to_fingerprint(
		const std::string &path,
		const image_descriptor &file,
		span<const std::size_t> indices
	) noexcept
	{
		m_fingerprint = hash_value(m_fingerprint, path.size());
		m_fingerprint =
			hash_bytes(m_fingerprint, as_bytes(path.data()), path.size());
		m_fingerprint = hash_value(
			m_fingerprint,
			static_cast<std::uint64_t>(file.get_data_type())
		);

		const auto extents = file.get_extents();
		m_fingerprint = hash_value(m_fingerprint, extents.size());
		for (const auto extent : extents)
		{
			m_fingerprint = hash_value(m_fingerprint, extent);
		}

		m_fingerprint = hash_value(m_fingerprint, indices.size());
		for (const auto index : indices)
		{
			m_fingerprint = hash_value(m_fingerprint, index);
		}
	}

	std::size_t m_capacity;
	std::size_t m_run_length;
	std::size_t m_used;
	std::size_t m_entry_count;
	std::uint64_t m_fingerprint;
	array_descriptor m_flags;
	array_descriptor m_values;
};

void check_not_empty(const storage_cursor &layout)
{
	if (layout.get_entry_count() == 0)
	{
		throw std::invalid_argument(
			"buffer_image_scratch: There is nothing to hold. The locations "
			"name no image, or the maximum size has no room for one."
		);
	}
}

// How a scratch of some locations lays out its storage, when it may use no
// more than a maximum. It is a dry run of the constructor: the files are
// taken and placed the same way, and nothing is stored.
storage_cursor lay_out(
	const image_location_grouping &locations,
	image_reader_provider &files,
	std::size_t run_length,
	std::size_t max_size
)
{
	const auto group_count = locations.get_group_count();

	storage_cursor cursor(max_size, run_length);
	for (std::size_t index = 0; index < group_count; ++index)
	{
		const auto group = locations.get_group(index);
		const auto &path = group.get_path();
		const auto file = files.acquire(path);
		REXLIB_ASSERT(file);

		const auto &descriptor = file->get_descriptor();
		const auto indices = get_named_indices(group, descriptor);
		if (indices.empty() || compute_slot_size(descriptor) == 0)
		{
			continue;
		}

		const auto held_count =
			cursor.place(path, descriptor, make_span(indices));
		if (held_count < indices.size())
		{
			break;
		}
	}

	return cursor;
}

// The storage of the file at a path, if that file holds a scratch that is
// laid out as `layout` says. Null otherwise.
std::shared_ptr<buffer>
open_storage(const std::string &path, const storage_cursor &layout)
{
	boost::system::error_code error;
	const auto size = boost::filesystem::file_size(path, error);
	if (error || size != layout.get_used())
	{
		return nullptr;
	}

	auto storage = open_mapped_file_buffer(path);
	if (read_fingerprint(*storage) != layout.get_fingerprint())
	{
		return nullptr;
	}

	return storage;
}

std::string make_temporary_path(const std::string &path)
{
	const auto suffix = boost::filesystem::unique_path("%%%%-%%%%-%%%%");
	return path + "." + suffix.string();
}

} // anonymous namespace

buffer_image_scratch::buffer_image_scratch(
	const image_location_grouping &locations,
	image_reader_provider &files,
	std::shared_ptr<buffer> storage,
	std::size_t run_length,
	image_scratch_open_mode mode
)
{
	check_storage(storage.get());
	check_run_length(run_length);

	const auto group_count = locations.get_group_count();

	std::vector<std::shared_ptr<buffer_image_scratch_entry>> entries;
	storage_cursor cursor(storage->get_size(), run_length);
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
			cursor.place(path, descriptor, make_span(indices));
		if (held_count > 0)
		{
			indices.resize(held_count);
			entries.push_back(
				std::make_shared<buffer_image_scratch_entry>(
					std::move(indices),
					array(storage, cursor.get_values()),
					array(storage, cursor.get_flags()),
					get_modification_time(path),
					run_length,
					mode
				)
			);
			m_entries.emplace(path, entries.back());
		}

		if (held_count < named_count)
		{
			break;
		}
	}

	// A resumed scratch has trusted its flags so far. They only mean
	// something if the storage was laid out like this one.
	const auto fingerprint = cursor.get_fingerprint();
	if (
		mode == image_scratch_open_mode::resumed &&
		read_fingerprint(*storage) != fingerprint
	)
	{
		for (const auto &entry : entries)
		{
			entry->reset();
		}
	}

	write_fingerprint(*storage, fingerprint);
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

	const auto layout = lay_out(locations, files, run_length, max_size);
	check_not_empty(layout);

	const auto allocator = get_host_memory_resource().create_allocator();

	return std::make_shared<buffer_image_scratch>(
		locations,
		files,
		allocator->allocate(
			layout.get_used(),
			allocator->get_max_alignment()
		),
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

	const auto layout = lay_out(locations, files, run_length, max_size);
	check_not_empty(layout);

	auto storage = open_storage(path, layout);
	if (storage)
	{
		return std::make_shared<buffer_image_scratch>(
			locations,
			files,
			std::move(storage),
			run_length,
			image_scratch_open_mode::resumed
		);
	}

	// Another program may have the file at the path mapped, so that file is
	// never changed. A new one is built beside it and takes its place.
	const auto temporary = make_temporary_path(path);
	try
	{
		auto scratch = std::make_shared<buffer_image_scratch>(
			locations,
			files,
			create_mapped_file_buffer(temporary, layout.get_used()),
			run_length,
			image_scratch_open_mode::empty
		);

		boost::system::error_code error;
		boost::filesystem::rename(temporary, path, error);
		if (error)
		{
			throw file_error(
				path + ": create_mapped_file_image_scratch: The scratch "
				"file could not be put in place: " + error.message()
			);
		}

		return scratch;
	}
	catch (...)
	{
		boost::system::error_code ignored;
		boost::filesystem::remove(temporary, ignored);
		throw;
	}
}

} // namespace em
} // namespace rexlib
