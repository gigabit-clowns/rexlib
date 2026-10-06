// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_read.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/counting_completion.hpp>
#include <rexlib/core/concurrency/executor.hpp>
#include <rexlib/core/concurrency/task.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/layout/index_table.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/clipping_image_transfer_sanitizer.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_loader.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_location_grouping.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_reader_provider.hpp>
#include <rexlib/em/image/image_scratch.hpp>
#include <rexlib/em/image/image_scratch_entry.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>
#include <rexlib/em/image/strict_image_transfer_sanitizer.hpp>
#include <rexlib/functional/creation.hpp>

#include <em/image/image_plan_builders.hpp>

#include <cstddef>
#include <memory>
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

image_transfer_plan make_indices_plan(
	const image_descriptor &file,
	span<const std::size_t> indices
)
{
	const auto extents = file.get_extents();
	const auto rank = extents.size();

	image_transfer_plan plan(
		image_transfer_shape(
			std::vector<std::size_t>(extents.begin() + 1, extents.end()),
			rank,
			rank - 1
		)
	);
	plan.reserve(indices.size());

	std::vector<std::size_t> file_offset(rank, 0);
	const std::vector<std::size_t> array_offset(rank - 1, 0);
	for (const auto index : indices)
	{
		file_offset.front() = index;
		plan.add(make_span(file_offset), make_span(array_offset));
	}

	return plan;
}

class scratch_prefetch_task final : public task
{
public:
	scratch_prefetch_task(
		std::string path,
		std::vector<std::size_t> indices,
		bool whole,
		std::shared_ptr<image_scratch_entry> entry,
		std::shared_ptr<image_reader_provider> files
	)
		: m_path(std::move(path))
		, m_indices(std::move(indices))
		, m_whole(whole)
		, m_entry(std::move(entry))
		, m_files(std::move(files))
	{
	}

	void run() override
	{
		const auto file = m_files->acquire(m_path);
		REXLIB_ASSERT(file);

		const auto &descriptor = file->get_descriptor();
		const auto plan = m_whole
			? make_location_plan(descriptor, image_location(m_path))
			: make_indices_plan(descriptor, make_span(m_indices));

		m_entry->store(*file, plan);
	}

private:
	std::string m_path;
	std::vector<std::size_t> m_indices;
	bool m_whole;
	std::shared_ptr<image_scratch_entry> m_entry;
	std::shared_ptr<image_reader_provider> m_files;
};

} // anonymous namespace

array read(
	const std::string &path,
	image_reader_provider &readers,
	const execution_context &context,
	numerical_type data_type
)
{
	return read(image_location(path), readers, context, data_type);
}

array read(
	const image_location &location,
	image_reader_provider &readers,
	const execution_context &context,
	numerical_type data_type
)
{
	const auto reader = readers.acquire(location.get_path());
	const auto &descriptor = reader->get_descriptor();
	const auto plan = make_location_plan(descriptor, location);

	auto destination = empty(
		make_contiguous_array_descriptor(
			plan.get_shape().get_extents(),
			data_type == numerical_type::unknown
				? descriptor.get_data_type()
				: data_type
		),
		memory_resource_affinity::host,
		context
	);

	array_ref destination_ref(destination);
	reader->read(destination_ref, plan);

	return destination;
}

std::shared_ptr<completion> read_batch_async(
	const image_loader &loader,
	array destination,
	span<const image_location> locations
)
{
	std::vector<std::size_t> array_extents;
	destination.get_descriptor().get_layout().get_extents(array_extents);

	const auto transaction =
		make_batch_plan(make_span(array_extents), locations);

	return loader.load(
		std::move(destination),
		transaction,
		strict_image_transfer_sanitizer::get_shared()
	);
}

std::shared_ptr<completion> read_patches_async(
	const image_loader &loader,
	array destination,
	const image_location &location,
	const index_table &centres
)
{
	std::vector<std::size_t> array_extents;
	destination.get_descriptor().get_layout().get_extents(array_extents);

	const auto transaction =
		make_patch_plan(make_span(array_extents), location, centres);

	return loader.load(
		std::move(destination),
		transaction,
		clipping_image_transfer_sanitizer::get_shared()
	);
}

std::shared_ptr<completion> prefetch_scratch_async(
	image_scratch &scratch,
	std::shared_ptr<image_reader_provider> files,
	rexlib::executor &executor,
	const image_location_grouping &locations
)
{
	if (!files)
	{
		throw std::invalid_argument(
			"prefetch_scratch_async: The reader provider must not be null."
		);
	}

	const auto file_count = locations.get_file_count();

	std::vector<std::unique_ptr<task>> tasks;
	for (std::size_t file_index = 0; file_index < file_count; ++file_index)
	{
		const auto &path = locations.get_path(file_index);
		auto entry = scratch.find(path);
		if (!entry)
		{
			continue;
		}

		const auto indices = locations.get_indices(file_index);
		tasks.push_back(
			std::make_unique<scratch_prefetch_task>(
				path,
				std::vector<std::size_t>(indices.begin(), indices.end()),
				locations.is_whole(file_index),
				std::move(entry),
				files
			)
		);
	}

	auto result = std::make_shared<counting_completion>(tasks.size());
	for (auto &prefetch : tasks)
	{
		executor.submit(std::move(prefetch), result);
	}

	return result;
}

} // namespace em
} // namespace rexlib
