// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_sink.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/counting_completion.hpp>
#include <rexlib/core/concurrency/executor.hpp>
#include <rexlib/core/concurrency/task.hpp>
#include <rexlib/core/ndarray/const_array.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_writer.hpp>
#include <rexlib/em/image/image_writer_provider.hpp>
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

class image_write_task final : public task
{
public:
	image_write_task(
		std::string path,
		image_transfer_plan transfer,
		std::shared_ptr<const_array> source,
		std::shared_ptr<image_writer_provider> writers
	)
		: m_path(std::move(path))
		, m_transfer(std::move(transfer))
		, m_source(std::move(source))
		, m_writers(std::move(writers))
	{
	}

	void run() override
	{
		const auto writer = m_writers->acquire(m_path);
		const_array_ref source(*m_source);
		writer->write(source, m_transfer);
	}

private:
	std::string m_path;
	image_transfer_plan m_transfer;
	std::shared_ptr<const_array> m_source;
	std::shared_ptr<image_writer_provider> m_writers;
};

} // namespace

image_sink::image_sink(
	std::shared_ptr<image_writer_provider> writers,
	std::shared_ptr<rexlib::executor> executor
)
	: m_writers(std::move(writers))
	, m_executor(std::move(executor))
{
	if (!m_writers)
	{
		throw std::invalid_argument(
			"image_sink: The writer provider must not be null."
		);
	}

	if (!m_executor)
	{
		throw std::invalid_argument(
			"image_sink: The executor must not be null."
		);
	}
}

image_sink::~image_sink() = default;

std::shared_ptr<completion> image_sink::write(
	const_array source,
	const image_transaction_plan &plan
)
{
	region_grouping grouping;
	grouping.build(plan);

	auto shared_source = std::make_shared<const_array>(std::move(source));
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
			std::make_unique<image_write_task>(
				plan.get_file(file_index),
				build_file_transfer_plan(plan, grouping, file_index),
				shared_source,
				m_writers
			),
			result
		);
	}

	return result;
}

void image_sink::flush()
{
	m_writers->flush();
}

} // namespace em
} // namespace rexlib
