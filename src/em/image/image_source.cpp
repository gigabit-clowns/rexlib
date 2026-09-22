// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_source.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/counting_completion.hpp>
#include <rexlib/core/concurrency/executor.hpp>
#include <rexlib/core/concurrency/task.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_reader_provider.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <em/image/image_region_clipping.hpp>
#include <em/image/image_region_grouping.hpp>

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

class image_read_task final : public task
{
public:
	image_read_task(
		std::string path,
		image_transfer_plan transfer,
		std::shared_ptr<array> destination,
		std::shared_ptr<image_reader_provider> readers
	)
		: m_path(std::move(path))
		, m_transfer(std::move(transfer))
		, m_destination(std::move(destination))
		, m_readers(std::move(readers))
	{
	}

	void run() override
	{
		const auto reader = m_readers->acquire(m_path);
		array_ref destination(*m_destination);

		std::vector<std::size_t> array_extents;
		destination.get_descriptor().get_layout().get_extents(array_extents);

		std::vector<image_transfer_plan> clipped;
		if (!make_clipped_transfer_plans(
				m_transfer,
				reader->get_extents(),
				make_span(array_extents),
				clipped
			))
		{
			reader->read(destination, m_transfer);
			return;
		}

		for (const auto &regions : clipped)
		{
			reader->read(destination, regions);
		}
	}

private:
	std::string m_path;
	image_transfer_plan m_transfer;
	std::shared_ptr<array> m_destination;
	std::shared_ptr<image_reader_provider> m_readers;
};

} // anonymous namespace

image_source::image_source(
	std::shared_ptr<image_reader_provider> readers,
	std::shared_ptr<rexlib::executor> executor
)
	: m_readers(std::move(readers))
	, m_executor(std::move(executor))
{
	if (!m_readers)
	{
		throw std::invalid_argument(
			"image_source: The reader provider must not be null."
		);
	}

	if (!m_executor)
	{
		throw std::invalid_argument(
			"image_source: The executor must not be null."
		);
	}
}

image_source::~image_source() = default;

std::shared_ptr<completion> image_source::read(
	array destination,
	const image_transaction_plan &plan
) const
{
	image_region_grouping grouping;
	grouping.build(plan);

	auto shared_destination = std::make_shared<array>(std::move(destination));
	auto result = std::make_shared<counting_completion>(
		grouping.get_addressed_file_count()
	);

	const auto file_count = grouping.get_file_count();
	for (std::size_t file_index = 0; file_index < file_count; ++file_index)
	{
		if (grouping.get_file_region_count(file_index) == 0)
		{
			continue;
		}

		m_executor->submit(
			std::make_unique<image_read_task>(
				plan.get_file(file_index),
				make_file_transfer_plan(grouping, plan, file_index),
				shared_destination,
				m_readers
			),
			result
		);
	}

	return result;
}

} // namespace em
} // namespace rexlib
