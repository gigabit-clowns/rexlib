// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/executor_image_loader.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/synchronous_executor.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include "../../core/concurrency/mock/mock_executor.hpp"
#include "../../core/hardware/mock/mock_buffer.hpp"
#include "mock/mock_image_reader.hpp"
#include "mock/mock_image_reader_provider.hpp"
#include "mock/mock_image_transfer_sanitizer.hpp"

#include <array>
#include <cstddef>
#include <functional>
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

// What a reader reports and what a destination carries, which is what the
// sanitizer is shown beside the regions of a file.
const std::vector<std::size_t> stack_extents = {8, 3, 5};
const std::vector<std::size_t> batch_extents = {4, 3, 5};

const image_descriptor stack_descriptor(
	make_span(stack_extents),
	plane_extents.size(),
	numerical_type::float32
);

array make_test_array()
{
	const auto storage = std::make_shared<mock_buffer>();
	const auto layout = strided_layout::make_contiguous_layout(
		make_span(batch_extents)
	);
	array_descriptor descriptor(layout, numerical_type::float32);
	return array(storage, std::move(descriptor));
}

// Add one whole element of a stack, as test_image_region_grouping does:
// element `index_in_stack` of file `file` lands in slot `slot` of a three
// dimensional array.
void add_element(
	image_transaction_plan &plan,
	std::size_t file,
	std::size_t index_in_stack,
	std::size_t slot
)
{
	const std::array<std::size_t, 3> file_offset = {index_in_stack, 0, 0};
	const std::array<std::size_t, 3> array_offset = {slot, 0, 0};
	plan.add(file, make_span(file_offset), make_span(array_offset));
}

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

// Answers every plan with itself, for the cases that are not about what a
// sanitizer answers.
std::unique_ptr<trompeloeil::expectation>
allow_passing_through(const mock_image_transfer_sanitizer &sanitizer)
{
	return NAMED_ALLOW_CALL(
		sanitizer,
		sanitize(trompeloeil::_, trompeloeil::_, trompeloeil::_)
	).RETURN( std::vector<image_transfer_plan>(1, _1) );
}

} // anonymous namespace

TEST_CASE(
	"executor_image_loader needs a reader provider and an executor",
	"[executor_image_loader]"
)
{
	SECTION( "a null reader provider" )
	{
		REQUIRE_THROWS_AS(
			executor_image_loader(
				nullptr,
				std::make_shared<synchronous_executor>()
			),
			std::invalid_argument
		);
	}

	SECTION( "a null executor" )
	{
		REQUIRE_THROWS_AS(
			executor_image_loader(
				std::make_shared<mock_image_reader_provider>(),
				nullptr
			),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"executor_image_loader needs a sanitizer",
	"[executor_image_loader]"
)
{
	const image_transaction_plan plan(
		image_transfer_shape(plane_extents, 3, 3)
	);

	executor_image_loader loader(
		std::make_shared<mock_image_reader_provider>(),
		std::make_shared<synchronous_executor>()
	);

	REQUIRE_THROWS_AS(
		loader.load(make_test_array(), plan, nullptr),
		std::invalid_argument
	);
}

TEST_CASE(
	"executor_image_loader reads each file's regions as one call, split by "
	"file",
	"[executor_image_loader]"
)
{
	// Three regions in one file, one in the other: exercises both the
	// many-regions and the few-regions skew in the same plan.
	image_transaction_plan plan(image_transfer_shape(plane_extents, 3, 3));
	const auto zero = plan.add_file("stack_0.mrcs");
	const auto one = plan.add_file("stack_1.mrcs");
	add_element(plan, zero, 0, 0);
	add_element(plan, zero, 1, 1);
	add_element(plan, zero, 2, 2);
	add_element(plan, one, 5, 3);

	const auto sanitizer = std::make_shared<mock_image_transfer_sanitizer>();
	const auto passing_through = allow_passing_through(*sanitizer);

	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader_zero = std::make_shared<mock_image_reader>();
	const auto reader_one = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("stack_0.mrcs")).RETURN(reader_zero);
	REQUIRE_CALL(*readers, acquire("stack_1.mrcs")).RETURN(reader_one);
	ALLOW_CALL(*reader_zero, get_descriptor())
		.RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(*reader_one, get_descriptor())
		.RETURN(std::ref(stack_descriptor));
	REQUIRE_CALL(*reader_zero, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( _2.get_region_count() == 3 );
	REQUIRE_CALL(*reader_one, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( _2.get_region_count() == 1 );

	executor_image_loader loader(
		readers,
		std::make_shared<synchronous_executor>()
	);
	const auto completion = loader.load(make_test_array(), plan, sanitizer);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"executor_image_loader does not acquire a reader for a file with no "
	"regions",
	"[executor_image_loader]"
)
{
	image_transaction_plan plan(image_transfer_shape(plane_extents, 3, 3));
	const auto zero = plan.add_file("stack_0.mrcs");
	plan.add_file("stack_1.mrcs"); // named, never given a region
	add_element(plan, zero, 0, 0);

	const auto sanitizer = std::make_shared<mock_image_transfer_sanitizer>();
	const auto passing_through = allow_passing_through(*sanitizer);

	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("stack_0.mrcs")).RETURN(reader);
	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_));
	// No expectation for "stack_1.mrcs": acquiring it would violate.

	executor_image_loader loader(
		readers,
		std::make_shared<synchronous_executor>()
	);
	const auto completion = loader.load(make_test_array(), plan, sanitizer);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"executor_image_loader resolves an empty plan without acquiring a reader",
	"[executor_image_loader]"
)
{
	const image_transaction_plan plan(
		image_transfer_shape(plane_extents, 3, 3)
	);

	// No expectations set on `readers` or on `sanitizer`: acquiring or
	// sanitizing anything would violate.
	const auto sanitizer = std::make_shared<mock_image_transfer_sanitizer>();
	const auto readers = std::make_shared<mock_image_reader_provider>();

	executor_image_loader loader(
		readers,
		std::make_shared<synchronous_executor>()
	);
	const auto completion = loader.load(make_test_array(), plan, sanitizer);

	REQUIRE( completion != nullptr );
	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"executor_image_loader's completion reports what a reader threw",
	"[executor_image_loader]"
)
{
	image_transaction_plan plan(image_transfer_shape(plane_extents, 3, 3));
	const auto zero = plan.add_file("stack_0.mrcs");
	add_element(plan, zero, 0, 0);

	const auto sanitizer = std::make_shared<mock_image_transfer_sanitizer>();
	const auto passing_through = allow_passing_through(*sanitizer);

	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("stack_0.mrcs")).RETURN(reader);
	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.SIDE_EFFECT( throw std::runtime_error("from a reader") );

	executor_image_loader loader(
		readers,
		std::make_shared<synchronous_executor>()
	);
	const auto completion = loader.load(make_test_array(), plan, sanitizer);

	REQUIRE( completion->is_ready() );
	REQUIRE_THROWS_AS( completion->get(), std::runtime_error );
}

TEST_CASE(
	"executor_image_loader shows each file's regions to the sanitizer beside "
	"the extents of both sides",
	"[executor_image_loader]"
)
{
	const std::vector<std::size_t> short_stack_extents = {2, 3, 5};
	const image_descriptor short_stack_descriptor(
		make_span(short_stack_extents),
		plane_extents.size(),
		numerical_type::float32
	);

	image_transaction_plan plan(image_transfer_shape(plane_extents, 3, 3));
	const auto zero = plan.add_file("stack_0.mrcs");
	const auto one = plan.add_file("stack_1.mrcs");
	add_element(plan, zero, 0, 0);
	add_element(plan, zero, 1, 1);
	add_element(plan, one, 1, 2);

	const auto sanitizer = std::make_shared<mock_image_transfer_sanitizer>();
	REQUIRE_CALL(
		*sanitizer,
		sanitize(trompeloeil::_, trompeloeil::_, trompeloeil::_)
	)
		.LR_WITH(
			_1.get_region_count() == 2 &&
			to_vector(_2) == stack_extents &&
			to_vector(_3) == batch_extents
		)
		.RETURN( std::vector<image_transfer_plan>(1, _1) );
	REQUIRE_CALL(
		*sanitizer,
		sanitize(trompeloeil::_, trompeloeil::_, trompeloeil::_)
	)
		.LR_WITH(
			_1.get_region_count() == 1 &&
			to_vector(_2) == short_stack_extents &&
			to_vector(_3) == batch_extents
		)
		.RETURN( std::vector<image_transfer_plan>(1, _1) );

	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader_zero = std::make_shared<mock_image_reader>();
	const auto reader_one = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("stack_0.mrcs")).RETURN(reader_zero);
	REQUIRE_CALL(*readers, acquire("stack_1.mrcs")).RETURN(reader_one);
	ALLOW_CALL(*reader_zero, get_descriptor())
		.RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(*reader_one, get_descriptor())
		.LR_RETURN(std::ref(short_stack_descriptor));
	REQUIRE_CALL(*reader_zero, read(trompeloeil::_, trompeloeil::_));
	REQUIRE_CALL(*reader_one, read(trompeloeil::_, trompeloeil::_));

	executor_image_loader loader(
		readers,
		std::make_shared<synchronous_executor>()
	);
	const auto completion = loader.load(make_test_array(), plan, sanitizer);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"executor_image_loader reads every plan the sanitizer answers with",
	"[executor_image_loader]"
)
{
	image_transaction_plan plan(image_transfer_shape(plane_extents, 3, 3));
	const auto zero = plan.add_file("stack_0.mrcs");
	add_element(plan, zero, 0, 0);

	// Two plans of other extents than the one asked for, which is how a
	// sanitizer answers when it shortens regions unevenly.
	const std::vector<image_transfer_plan> answered = {
		image_transfer_plan(
			image_transfer_shape(std::vector<std::size_t>{2, 5}, 3, 3)
		),
		image_transfer_plan(
			image_transfer_shape(std::vector<std::size_t>{1, 5}, 3, 3)
		)
	};

	const auto sanitizer = std::make_shared<mock_image_transfer_sanitizer>();
	REQUIRE_CALL(
		*sanitizer,
		sanitize(trompeloeil::_, trompeloeil::_, trompeloeil::_)
	)
		.LR_RETURN( answered );

	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("stack_0.mrcs")).RETURN(reader);
	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( _2.get_shape().get_extents()[0] == 2 );
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( _2.get_shape().get_extents()[0] == 1 );

	executor_image_loader loader(
		readers,
		std::make_shared<synchronous_executor>()
	);
	const auto completion = loader.load(make_test_array(), plan, sanitizer);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"executor_image_loader reads nothing of a file the sanitizer answers no "
	"plan for",
	"[executor_image_loader]"
)
{
	image_transaction_plan plan(image_transfer_shape(plane_extents, 3, 3));
	const auto zero = plan.add_file("stack_0.mrcs");
	add_element(plan, zero, 0, 0);

	const auto sanitizer = std::make_shared<mock_image_transfer_sanitizer>();
	REQUIRE_CALL(
		*sanitizer,
		sanitize(trompeloeil::_, trompeloeil::_, trompeloeil::_)
	)
		.RETURN( std::vector<image_transfer_plan>() );

	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("stack_0.mrcs")).RETURN(reader);
	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	// No expectation for `read`: reading anything would violate.

	executor_image_loader loader(
		readers,
		std::make_shared<synchronous_executor>()
	);
	const auto completion = loader.load(make_test_array(), plan, sanitizer);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"executor_image_loader's completion reports what the sanitizer threw",
	"[executor_image_loader]"
)
{
	image_transaction_plan plan(image_transfer_shape(plane_extents, 3, 3));
	const auto zero = plan.add_file("stack_0.mrcs");
	add_element(plan, zero, 0, 0);

	const auto sanitizer = std::make_shared<mock_image_transfer_sanitizer>();
	REQUIRE_CALL(
		*sanitizer,
		sanitize(trompeloeil::_, trompeloeil::_, trompeloeil::_)
	)
		.THROW( std::out_of_range("from a sanitizer") );

	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("stack_0.mrcs")).RETURN(reader);
	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	// No expectation for `read`: reading anything would violate.

	executor_image_loader loader(
		readers,
		std::make_shared<synchronous_executor>()
	);
	const auto completion = loader.load(make_test_array(), plan, sanitizer);

	REQUIRE( completion->is_ready() );
	REQUIRE_THROWS_AS( completion->get(), std::out_of_range );
}

TEST_CASE(
	"executor_image_loader submits each file as a task of its own",
	"[executor_image_loader]"
)
{
	// Running the tasks, and how many at once, is the executor's business.
	// The loader's is to submit one task per file and wait for none of them.
	static REXLIB_CONST_CONSTEXPR std::size_t file_count = 4;

	image_transaction_plan plan(image_transfer_shape(plane_extents, 3, 3));
	for (std::size_t i = 0; i < file_count; ++i)
	{
		const auto file = plan.add_file("stack_" + std::to_string(i) + ".mrcs");
		add_element(plan, file, 0, i);
	}

	// No expectations set on `readers`: reading is what the tasks do.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto executor = std::make_shared<mock_executor>();

	REQUIRE_CALL(*executor, submit(trompeloeil::_, trompeloeil::_))
		.WITH( _1 != nullptr && _2 != nullptr )
		.TIMES(file_count);

	// No expectations set on `sanitizer` either: no task runs.
	const auto sanitizer = std::make_shared<mock_image_transfer_sanitizer>();

	executor_image_loader loader(readers, executor);
	const auto completion = loader.load(make_test_array(), plan, sanitizer);

	CHECK_FALSE( completion->is_ready() );
}

TEST_CASE(
	"executor_image_loader lets a second read start before the first ends",
	"[executor_image_loader]"
)
{
	image_transaction_plan first_plan(
		image_transfer_shape(plane_extents, 3, 3)
	);
	const auto first_file = first_plan.add_file("stack_0.mrcs");
	add_element(first_plan, first_file, 0, 0);

	image_transaction_plan second_plan(
		image_transfer_shape(plane_extents, 3, 3)
	);
	const auto second_file = second_plan.add_file("stack_1.mrcs");
	add_element(second_plan, second_file, 0, 0);

	// No expectations set on `readers`: no task runs, so none reads.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto executor = std::make_shared<mock_executor>();

	REQUIRE_CALL(*executor, submit(trompeloeil::_, trompeloeil::_))
		.TIMES(2);

	const auto sanitizer = std::make_shared<mock_image_transfer_sanitizer>();

	executor_image_loader loader(readers, executor);
	const auto first = loader.load(make_test_array(), first_plan, sanitizer);
	const auto second =
		loader.load(make_test_array(), second_plan, sanitizer);

	CHECK_FALSE( first->is_ready() );
	CHECK_FALSE( second->is_ready() );
}
