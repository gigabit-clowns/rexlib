// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/executor_image_sink.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/synchronous_executor.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>

#include "../../core/concurrency/mock/mock_executor.hpp"
#include "../../core/hardware/mock/mock_buffer.hpp"
#include "mock/mock_image_writer.hpp"
#include "mock/mock_image_writer_provider.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <trompeloeil.hpp>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

const std::vector<std::size_t> plane_extents = {3, 5};

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

// Add one whole element of a stack, as test_image_region_grouping does:
// element `index_in_stack` of file `file` comes from slot `slot` of a three
// dimensional array.
void add_element(
	image_transaction_plan &plan,
	std::size_t file,
	std::size_t index_in_stack,
	std::size_t slot
)
{
	const std::size_t file_offset[3] = {index_in_stack, 0, 0};
	const std::size_t array_offset[3] = {slot, 0, 0};
	plan.add(file, make_span(file_offset, 3), make_span(array_offset, 3));
}

} // anonymous namespace

TEST_CASE(
	"executor_image_sink needs a writer provider and an executor",
	"[executor_image_sink]"
)
{
	SECTION( "a null writer provider" )
	{
		REQUIRE_THROWS_AS(
			executor_image_sink(
				nullptr,
				std::make_shared<synchronous_executor>()
			),
			std::invalid_argument
		);
	}

	SECTION( "a null executor" )
	{
		REQUIRE_THROWS_AS(
			executor_image_sink(
				std::make_shared<mock_image_writer_provider>(),
				nullptr
			),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"executor_image_sink writes each file's regions as one call, split by file",
	"[executor_image_sink]"
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
		.LR_WITH( _2.get_region_count() == 3 );
	REQUIRE_CALL(*writer_one, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH( _2.get_region_count() == 1 );

	executor_image_sink sink(writers, std::make_shared<synchronous_executor>());
	const auto completion = sink.write(make_test_array(), plan);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"executor_image_sink does not acquire a writer for a file with no regions",
	"[executor_image_sink]"
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

	executor_image_sink sink(writers, std::make_shared<synchronous_executor>());
	const auto completion = sink.write(make_test_array(), plan);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"executor_image_sink resolves an empty plan without acquiring a writer",
	"[executor_image_sink]"
)
{
	const image_transaction_plan plan(make_span(plane_extents), 3, 3);

	// No expectations set on `writers`: acquiring anything would violate.
	const auto writers = std::make_shared<mock_image_writer_provider>();

	executor_image_sink sink(writers, std::make_shared<synchronous_executor>());
	const auto completion = sink.write(make_test_array(), plan);

	REQUIRE( completion != nullptr );
	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"executor_image_sink's completion reports what a writer threw",
	"[executor_image_sink]"
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

	executor_image_sink sink(writers, std::make_shared<synchronous_executor>());
	const auto completion = sink.write(make_test_array(), plan);

	REQUIRE( completion->is_ready() );
	REQUIRE_THROWS_AS( completion->get(), std::runtime_error );
}

TEST_CASE(
	"executor_image_sink submits each file as a task of its own",
	"[executor_image_sink]"
)
{
	// Running the tasks, and how many at once, is the executor's business.
	// The sink's is to submit one task per file and wait for none of them.
	static REXLIB_CONST_CONSTEXPR std::size_t file_count = 4;

	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	for (std::size_t i = 0; i < file_count; ++i)
	{
		const auto file = plan.add_file("stack_" + std::to_string(i) + ".mrcs");
		add_element(plan, file, 0, i);
	}

	// No expectations set on `writers`: writing is what the tasks do.
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto executor = std::make_shared<mock_executor>();

	REQUIRE_CALL(*executor, submit(trompeloeil::_, trompeloeil::_))
		.WITH( _1 != nullptr && _2 != nullptr )
		.TIMES(file_count);

	executor_image_sink sink(writers, executor);
	const auto completion = sink.write(make_test_array(), plan);

	CHECK_FALSE( completion->is_ready() );
}

TEST_CASE(
	"executor_image_sink's flush delegates to the writer provider",
	"[executor_image_sink]"
)
{
	const auto writers = std::make_shared<mock_image_writer_provider>();
	REQUIRE_CALL(*writers, flush());

	executor_image_sink sink(writers, std::make_shared<synchronous_executor>());
	sink.flush();
}
