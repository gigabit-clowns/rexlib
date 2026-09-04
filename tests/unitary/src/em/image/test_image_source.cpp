// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_source.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/synchronous_executor.hpp>
#include <rexlib/core/concurrency/thread_pool_executor.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>

#include "../../core/hardware/mock/mock_buffer.hpp"
#include "mock/mock_image_reader.hpp"
#include "mock/mock_image_reader_provider.hpp"

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
// concurrency tests below therefore use plain fakes instead, for both the
// provider and the readers.

// Filled once, before any task runs, and never written to again, so
// concurrent acquire() calls are concurrent reads of an otherwise-immutable
// map.
class fixed_image_reader_provider final : public image_reader_provider
{
public:
	explicit fixed_image_reader_provider(
		std::unordered_map<std::string, std::shared_ptr<const image_reader>>
			readers
	)
		: m_readers(std::move(readers))
	{
	}

	std::shared_ptr<const image_reader>
	acquire(const std::string &path) override
	{
		return m_readers.at(path);
	}

private:
	const std::unordered_map<std::string, std::shared_ptr<const image_reader>>
		m_readers;
};

// A reader whose only behaviour is to run a callback from read(). The other
// methods are never called by image_source and only exist to satisfy
// image_reader's interface.
class barrier_image_reader final : public image_reader
{
public:
	explicit barrier_image_reader(std::function<void()> on_read)
		: m_on_read(std::move(on_read))
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

	const image_metadata& get_metadata() const noexcept override
	{
		return m_metadata;
	}

	void read(array_ref, const image_transfer_plan &) const override
	{
		m_on_read();
	}

private:
	std::function<void()> m_on_read;
	image_metadata m_metadata;
};

array make_test_array()
{
	const std::vector<std::size_t> extents = {1};
	const auto storage = std::make_shared<mock_buffer>();
	const auto layout = strided_layout::make_contiguous_layout(
		make_span(extents)
	);
	array_descriptor descriptor(layout, numerical_type::float32);
	return array(storage, std::move(descriptor));
}

// Add one whole element of a stack: element `position` of file `file` lands
// in slot `slot` of a three dimensional array. Mirrors test_region_grouping.
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
	"image_source needs a reader provider and an executor",
	"[image_source]"
)
{
	SECTION( "a null reader provider" )
	{
		REQUIRE_THROWS_AS(
			image_source(nullptr, std::make_shared<synchronous_executor>()),
			std::invalid_argument
		);
	}

	SECTION( "a null executor" )
	{
		REQUIRE_THROWS_AS(
			image_source(
				std::make_shared<mock_image_reader_provider>(),
				nullptr
			),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"image_source reads each file's regions as one call, split by file",
	"[image_source]"
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

	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader_zero = std::make_shared<mock_image_reader>();
	const auto reader_one = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("stack_0.mrcs")).RETURN(reader_zero);
	REQUIRE_CALL(*readers, acquire("stack_1.mrcs")).RETURN(reader_one);
	REQUIRE_CALL(*reader_zero, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( _2.get_size() == 3 );
	REQUIRE_CALL(*reader_one, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( _2.get_size() == 1 );

	image_source source(readers, std::make_shared<synchronous_executor>());
	const auto completion = source.read(make_test_array(), plan);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_source does not acquire a reader for a file with no regions",
	"[image_source]"
)
{
	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	const auto zero = plan.add_file("stack_0.mrcs");
	plan.add_file("stack_1.mrcs"); // named, never given a region
	add_element(plan, zero, 0, 0);

	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("stack_0.mrcs")).RETURN(reader);
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_));
	// No expectation for "stack_1.mrcs": acquiring it would violate.

	image_source source(readers, std::make_shared<synchronous_executor>());
	const auto completion = source.read(make_test_array(), plan);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_source's completion reports what a reader threw",
	"[image_source]"
)
{
	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	const auto zero = plan.add_file("stack_0.mrcs");
	add_element(plan, zero, 0, 0);

	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("stack_0.mrcs")).RETURN(reader);
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.SIDE_EFFECT( throw std::runtime_error("from a reader") );

	image_source source(readers, std::make_shared<synchronous_executor>());
	const auto completion = source.read(make_test_array(), plan);

	REQUIRE( completion->is_ready() );
	REQUIRE_THROWS_AS( completion->get(), std::runtime_error );
}

TEST_CASE(
	"image_source reads the files of one transaction concurrently",
	"[image_source]"
)
{
	// Each file's read spins until every other one has also started. A
	// source that serialized file reads would deadlock the first one here
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

	std::unordered_map<std::string, std::shared_ptr<const image_reader>>
		readers_by_path;
	for (std::size_t i = 0; i < file_count; ++i)
	{
		const auto path = "stack_" + std::to_string(i) + ".mrcs";
		const auto file = plan.add_file(path);
		add_element(plan, file, 0, i);

		readers_by_path.emplace(
			path,
			std::make_shared<barrier_image_reader>(barrier)
		);
	}
	const auto readers = std::make_shared<fixed_image_reader_provider>(
		std::move(readers_by_path)
	);

	image_source source(
		readers,
		std::make_shared<thread_pool_executor>(file_count)
	);
	const auto completion = source.read(make_test_array(), plan);

	completion->wait();

	CHECK( entered.load() == file_count );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_source lets two concurrently outstanding reads interleave",
	"[image_source]"
)
{
	REXLIB_CONST_CONSTEXPR std::size_t transaction_count = 2;

	image_transaction_plan first_plan(make_span(plane_extents), 3, 3);
	const auto first_file = first_plan.add_file("stack_0.mrcs");
	add_element(first_plan, first_file, 0, 0);

	image_transaction_plan second_plan(make_span(plane_extents), 3, 3);
	const auto second_file = second_plan.add_file("stack_1.mrcs");
	add_element(second_plan, second_file, 0, 0);

	std::atomic<std::size_t> entered(0);
	const auto barrier =
		[&entered]
		{
			entered.fetch_add(1);
			while (entered.load() < transaction_count)
			{
				std::this_thread::yield();
			}
		};

	const auto readers = std::make_shared<fixed_image_reader_provider>(
		std::unordered_map<std::string, std::shared_ptr<const image_reader>>{
			{"stack_0.mrcs", std::make_shared<barrier_image_reader>(barrier)},
			{"stack_1.mrcs", std::make_shared<barrier_image_reader>(barrier)}
		}
	);

	image_source source(
		readers,
		std::make_shared<thread_pool_executor>(transaction_count)
	);
	const auto first_completion = source.read(make_test_array(), first_plan);
	const auto second_completion =
		source.read(make_test_array(), second_plan);

	first_completion->wait();
	second_completion->wait();

	CHECK( entered.load() == transaction_count );
	CHECK_NOTHROW( first_completion->get() );
	CHECK_NOTHROW( second_completion->get() );
}
