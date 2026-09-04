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
#include <rexlib/em/image/region_grouping.hpp>

#include <em/image/region_dispatch.hpp>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

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
		reader->read(destination, m_transfer);
	}

private:
	std::string m_path;
	image_transfer_plan m_transfer;
	std::shared_ptr<array> m_destination;
	std::shared_ptr<image_reader_provider> m_readers;
};

} // namespace

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
	region_grouping grouping;
	grouping.build(plan);

	auto shared_destination = std::make_shared<array>(std::move(destination));
	auto result = std::make_shared<counting_completion>(
		count_files_with_regions(grouping)
	);

	for (
		std::size_t file_index = 0;
		file_index < grouping.get_file_count();
		++file_index
	)
	{
		if (grouping.get_region_count(file_index) == 0)
		{
			continue;
		}

		m_executor->submit(
			std::make_unique<image_read_task>(
				plan.get_file(file_index),
				build_file_transfer_plan(plan, grouping, file_index),
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
