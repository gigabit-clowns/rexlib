// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_sink.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/synchronous_executor.hpp>
#include <rexlib/core/concurrency/thread_pool_executor.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>

#include "../../core/hardware/mock/mock_buffer.hpp"
#include "mock/mock_image_writer.hpp"
#include "mock/mock_image_writer_provider.hpp"

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <trompeloeil.hpp>
#include <unordered_map>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

const std::vector<std::size_t> plane_extents = {3, 5};

// trompeloeil keeps no internal lock, so a mock is not safe to call from
// several threads at once — not just on the same instance, since matching a
// call also touches bookkeeping trompeloeil shares across every mock. The
// concurrency test below therefore uses plain fakes instead, for both the
// provider and the writers.

// Filled once, before any task runs, and never written to again, so
// concurrent acquire() calls are concurrent reads of an otherwise-immutable
// map.
class fixed_image_writer_provider final : public image_writer_provider
{
public:
	explicit fixed_image_writer_provider(
		std::unordered_map<std::string, std::shared_ptr<image_writer>>
			writers
	)
		: m_writers(std::move(writers))
	{
	}

	std::shared_ptr<image_writer> acquire(const std::string &path) override
	{
		return m_writers.at(path);
	}

	void flush() override
	{
	}

private:
	const std::unordered_map<std::string, std::shared_ptr<image_writer>>
		m_writers;
};

// A writer whose only behaviour is to run a callback from write(). The other
// methods are never called by image_sink and only exist to satisfy
// image_writer's interface.
class barrier_image_writer final : public image_writer
{
public:
	explicit barrier_image_writer(std::function<void()> on_write)
		: m_on_write(std::move(on_write))
	{
	}

	span<const std::size_t> get_extents() const noexcept override
	{
		return span<const std::size_t>();
	}

	std::size_t get_core_rank() const noexcept override
	{
		return 0;
	}

	numerical_type get_data_type() const noexcept override
	{
		return numerical_type::float32;
	}

	void write(const_array_ref, const image_transfer_plan &) override
	{
		m_on_write();
	}

	void flush() override
	{
	}

private:
	std::function<void()> m_on_write;
};

const_array make_test_array()
{
	const std::vector<std::size_t> extents = {1};
	const auto storage = std::make_shared<mock_buffer>();
	const auto layout = strided_layout::make_contiguous_layout(
		make_span(extents)
	);
	array_descriptor descriptor(layout, numerical_type::float32);
	return array(storage, std::move(descriptor)).share_const();
}

// Add one whole element of a stack: element `position` of file `file` comes
// from slot `slot` of a three dimensional array. Mirrors test_region_grouping.
void add_element(
	image_transaction_plan &plan,
	std::size_t file,
	std::size_t position,
	std::size_t slot
)
{
	const std::size_t file_offset[3] = {position, 0, 0};
	const std::size_t array_offset[3] = {slot, 0, 0};
	plan.add(file, make_span(file_offset, 3), make_span(array_offset, 3));
}

} // namespace

TEST_CASE(
	"image_sink needs a writer provider and an executor",
	"[image_sink]"
)
{
	SECTION( "a null writer provider" )
	{
		REQUIRE_THROWS_AS(
			image_sink(nullptr, std::make_shared<synchronous_executor>()),
			std::invalid_argument
		);
	}

	SECTION( "a null executor" )
	{
		REQUIRE_THROWS_AS(
			image_sink(
				std::make_shared<mock_image_writer_provider>(),
				nullptr
			),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"image_sink writes each file's regions as one call, split by file",
	"[image_sink]"
)
{
	// Three regions in one file, one in the other: exercises both the
	// many-regions and the few-regions skew in the same plan.
	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	const auto zero = plan.add_file("stack_0.mrcs");
	const auto one = plan.add_file("stack_1.mrcs");
	add_element(plan, zero, 0, 0);
	add_element(plan, zero, 1, 1);
	add_element(plan, zero, 2, 2);
	add_element(plan, one, 5, 3);

	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer_zero = std::make_shared<mock_image_writer>();
	const auto writer_one = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("stack_0.mrcs")).RETURN(writer_zero);
	REQUIRE_CALL(*writers, acquire("stack_1.mrcs")).RETURN(writer_one);
	REQUIRE_CALL(*writer_zero, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH( _2.get_size() == 3 );
	REQUIRE_CALL(*writer_one, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH( _2.get_size() == 1 );

	image_sink sink(writers, std::make_shared<synchronous_executor>());
	const auto completion = sink.write(make_test_array(), plan);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_sink does not acquire a writer for a file with no regions",
	"[image_sink]"
)
{
	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	const auto zero = plan.add_file("stack_0.mrcs");
	plan.add_file("stack_1.mrcs"); // named, never given a region
	add_element(plan, zero, 0, 0);

	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("stack_0.mrcs")).RETURN(writer);
	REQUIRE_CALL(*writer, write(trompeloeil::_, trompeloeil::_));
	// No expectation for "stack_1.mrcs": acquiring it would violate.

	image_sink sink(writers, std::make_shared<synchronous_executor>());
	const auto completion = sink.write(make_test_array(), plan);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_sink's completion reports what a writer threw",
	"[image_sink]"
)
{
	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	const auto zero = plan.add_file("stack_0.mrcs");
	add_element(plan, zero, 0, 0);

	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("stack_0.mrcs")).RETURN(writer);
	REQUIRE_CALL(*writer, write(trompeloeil::_, trompeloeil::_))
		.SIDE_EFFECT( throw std::runtime_error("from a writer") );

	image_sink sink(writers, std::make_shared<synchronous_executor>());
	const auto completion = sink.write(make_test_array(), plan);

	REQUIRE( completion->is_ready() );
	REQUIRE_THROWS_AS( completion->get(), std::runtime_error );
}

TEST_CASE(
	"image_sink writes the files of one transaction concurrently",
	"[image_sink]"
)
{
	// Each file's write spins until every other one has also started. A
	// sink that serialized file writes would deadlock the first one here
	// rather than merely run slowly.
	REXLIB_CONST_CONSTEXPR std::size_t file_count = 4;

	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	std::atomic<std::size_t> entered(0);
	const auto barrier =
		[&entered]
		{
			entered.fetch_add(1);
			while (entered.load() < file_count)
			{
				std::this_thread::yield();
			}
		};

	std::unordered_map<std::string, std::shared_ptr<image_writer>>
		writers_by_path;
	for (std::size_t i = 0; i < file_count; ++i)
	{
		const auto path = "stack_" + std::to_string(i) + ".mrcs";
		const auto file = plan.add_file(path);
		add_element(plan, file, 0, i);

		writers_by_path.emplace(
			path,
			std::make_shared<barrier_image_writer>(barrier)
		);
	}
	const auto writers = std::make_shared<fixed_image_writer_provider>(
		std::move(writers_by_path)
	);

	image_sink sink(
		writers,
		std::make_shared<thread_pool_executor>(file_count)
	);
	const auto completion = sink.write(make_test_array(), plan);

	completion->wait();

	CHECK( entered.load() == file_count );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE( "image_sink's flush delegates to the writer provider", "[image_sink]" )
{
	const auto writers = std::make_shared<mock_image_writer_provider>();
	REQUIRE_CALL(*writers, flush());

	image_sink sink(writers, std::make_shared<synchronous_executor>());
	sink.flush();
}
