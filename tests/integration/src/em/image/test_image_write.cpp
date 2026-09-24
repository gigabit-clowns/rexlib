// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_write.hpp>

#include "../../functional/fixtures/cpu_execution_context_fixture.hpp"
#include "fixtures/scoped_path.hpp"

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/synchronous_executor.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/ndarray/const_array.hpp>
#include <rexlib/em/image/direct_image_reader_provider.hpp>
#include <rexlib/em/image/executor_image_sink.hpp>
#include <rexlib/em/image/executor_image_source.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_read.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>
#include <rexlib/em/image/managed_image_writer_provider.hpp>
#include <rexlib/functional/creation.hpp>

#include <cstddef>
#include <cstring>
#include <memory>
#include <numeric>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// A stack of six images of three by four, written and read back two at a
// time, so that neither end sees the stack whole.
const std::size_t stack_count = 6;
const std::size_t batch_size = 2;
const std::vector<std::size_t> stack_extents = {stack_count, 3, 4};
const std::vector<std::size_t> batch_extents = {batch_size, 3, 4};
const std::size_t core_rank = 2;

std::size_t element_count(const std::vector<std::size_t> &extents)
{
	return std::accumulate(
		extents.cbegin(),
		extents.cend(),
		std::size_t(1),
		std::multiplies<std::size_t>()
	);
}

std::vector<float> counting(std::size_t count)
{
	std::vector<float> values(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		values[i] = static_cast<float>(i) + 0.25F;
	}

	return values;
}

// The slots one batch covers, which is what a caller writing a stack a
// batch at a time names.
std::vector<image_location> slots_of(
	const std::string &path,
	std::size_t batch_index
)
{
	std::vector<image_location> slots;
	slots.reserve(batch_size);
	for (std::size_t i = 0; i < batch_size; ++i)
	{
		slots.emplace_back(path, batch_index * batch_size + i);
	}

	return slots;
}

} // anonymous namespace

TEST_CASE_METHOD( cpu_execution_context_fixture,
	"a stack written a batch at a time reads back a batch at a time",
	"[mrc][image_write]" )
{
	const scoped_path path("batch_sink_stack.mrcs");
	const auto values = counting(element_count(stack_extents));
	const auto batch_elements = element_count(batch_extents);
	const auto batch_count = stack_count / batch_size;

	const auto writer_formats =
		catalog.get_service_manager<image_write_format_manager>();
	const auto writers =
		std::make_shared<managed_image_writer_provider>(writer_formats);
	const auto sink = std::make_shared<executor_image_sink>(
		writers,
		std::make_shared<synchronous_executor>()
	);

	// The size of the stack is settled here and nowhere else.
	const image_descriptor stack_descriptor(
		make_span(stack_extents),
		core_rank,
		numerical_type::float32
	);
	writers->declare(path.get(), stack_descriptor, image_metadata());

	std::vector<std::shared_ptr<completion>> written;
	for (std::size_t k = 0; k < batch_count; ++k)
	{
		auto source = zeros(
			make_descriptor(batch_extents, numerical_type::float32),
			memory_resource_affinity::host,
			context
		);
		std::memcpy(
			source.get_storage()->get_host_ptr(),
			values.data() + k * batch_elements,
			batch_elements * sizeof(float)
		);

		const auto slots = slots_of(path.get(), k);
		written.push_back(
			write_batch_async(*sink, source.share_const(), make_span(slots))
		);
	}

	for (const auto &completion : written)
	{
		REQUIRE_NOTHROW( completion->get() );
	}

	// Only once every batch has landed, which is what close's contract asks
	// of a caller.
	writers->close(path.get());

	const auto reader_formats =
		catalog.get_service_manager<image_read_format_manager>();
	const auto readers =
		std::make_shared<direct_image_reader_provider>(reader_formats);
	const auto source = std::make_shared<executor_image_source>(
		readers,
		std::make_shared<synchronous_executor>()
	);

	SECTION( "the file states the stack it was declared as" )
	{
		const auto reader = reader_formats->open(path.get());

		CHECK( reader->get_descriptor() == stack_descriptor );
	}

	SECTION( "every batch holds what was written into its slots" )
	{
		for (std::size_t k = 0; k < batch_count; ++k)
		{
			auto destination = zeros(
				make_descriptor(batch_extents, numerical_type::float32),
				memory_resource_affinity::host,
				context
			);

			const auto slots = slots_of(path.get(), k);
			const auto completion = read_batch_async(
				*source,
				destination.share(),
				make_span(slots)
			);
			REQUIRE_NOTHROW( completion->get() );

			const std::vector<float> expected(
				values.begin() + k * batch_elements,
				values.begin() + (k + 1) * batch_elements
			);

			CHECK( read_host<float>(destination, batch_elements) == expected );
		}
	}
}
