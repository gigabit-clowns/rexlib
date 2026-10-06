// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_scratch.hpp>

#include "image_location_grouping.hpp"
#include "image_plan_builders.hpp"

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/counting_completion.hpp>
#include <rexlib/core/concurrency/executor.hpp>
#include <rexlib/core/concurrency/task.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_reader_provider.hpp>
#include <rexlib/em/image/image_scratch_entry.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <cstddef>
#include <stdexcept>
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

image_scratch::image_scratch() noexcept = default;
image_scratch::~image_scratch() = default;

std::shared_ptr<completion> prefetch_scratch_async(
	image_scratch &scratch,
	std::shared_ptr<image_reader_provider> files,
	rexlib::executor &executor,
	span<const image_location> locations
)
{
	if (!files)
	{
		throw std::invalid_argument(
			"prefetch_scratch_async: The reader provider must not be null."
		);
	}

	const image_location_grouping grouping(locations);
	const auto file_count = grouping.get_file_count();

	std::vector<std::unique_ptr<task>> tasks;
	for (std::size_t file_index = 0; file_index < file_count; ++file_index)
	{
		const auto &path = grouping.get_path(file_index);
		auto entry = scratch.find(path);
		if (!entry)
		{
			continue;
		}

		const auto indices = grouping.get_indices(file_index);
		tasks.push_back(
			std::make_unique<scratch_prefetch_task>(
				path,
				std::vector<std::size_t>(indices.begin(), indices.end()),
				grouping.is_whole(file_index),
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
