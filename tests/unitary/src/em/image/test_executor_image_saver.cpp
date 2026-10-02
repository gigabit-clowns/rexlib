// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/executor_image_saver.hpp>

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
#include "mock/mock_image_transfer_sanitizer.hpp"
#include "mock/mock_image_writer.hpp"
#include "mock/mock_image_writer_provider.hpp"

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

// What a writer reports and what a source carries, which is what the
// sanitizer is shown beside the regions of a file.
const std::vector<std::size_t> stack_extents = {8, 3, 5};
const std::vector<std::size_t> batch_extents = {4, 3, 5};

const image_descriptor stack_descriptor(
	make_span(stack_extents),
	plane_extents.size(),
	numerical_type::float32
);

const_array make_test_array()
{
	const auto storage = std::make_shared<mock_buffer>();
	const auto layout = strided_layout::make_contiguous_layout(
		make_span(batch_extents)
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
	"executor_image_saver needs a writer provider and an executor",
	"[executor_image_saver]"
)
{
	SECTION( "a null writer provider" )
	{
		REQUIRE_THROWS_AS(
			executor_image_saver(
				nullptr,
				std::make_shared<synchronous_executor>()
			),
			std::invalid_argument
		);
	}

	SECTION( "a null executor" )
	{
		REQUIRE_THROWS_AS(
			executor_image_saver(
				std::make_shared<mock_image_writer_provider>(),
				nullptr
			),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"executor_image_saver needs a sanitizer",
	"[executor_image_saver]"
)
{
	const image_transaction_plan plan(
		image_transfer_shape(plane_extents, 3, 3)
	);

	executor_image_saver saver(
		std::make_shared<mock_image_writer_provider>(),
		std::make_shared<synchronous_executor>()
	);

	REQUIRE_THROWS_AS(
		saver.save(make_test_array(), plan, nullptr),
		std::invalid_argument
	);
}

TEST_CASE(
	"executor_image_saver writes each file's regions as one call, split by "
	"file",
	"[executor_image_saver]"
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

	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer_zero = std::make_shared<mock_image_writer>();
	const auto writer_one = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("stack_0.mrcs")).RETURN(writer_zero);
	REQUIRE_CALL(*writers, acquire("stack_1.mrcs")).RETURN(writer_one);
	ALLOW_CALL(*writer_zero, get_descriptor())
		.RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(*writer_one, get_descriptor())
		.RETURN(std::ref(stack_descriptor));
	REQUIRE_CALL(*writer_zero, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH( _2.get_region_count() == 3 );
	REQUIRE_CALL(*writer_one, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH( _2.get_region_count() == 1 );

	executor_image_saver saver(
		writers,
		std::make_shared<synchronous_executor>()
	);
	const auto completion = saver.save(make_test_array(), plan, sanitizer);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"executor_image_saver does not acquire a writer for a file with no regions",
	"[executor_image_saver]"
)
{
	image_transaction_plan plan(image_transfer_shape(plane_extents, 3, 3));
	const auto zero = plan.add_file("stack_0.mrcs");
	plan.add_file("stack_1.mrcs"); // named, never given a region
	add_element(plan, zero, 0, 0);

	const auto sanitizer = std::make_shared<mock_image_transfer_sanitizer>();
	const auto passing_through = allow_passing_through(*sanitizer);

	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("stack_0.mrcs")).RETURN(writer);
	ALLOW_CALL(*writer, get_descriptor()).RETURN(std::ref(stack_descriptor));
	REQUIRE_CALL(*writer, write(trompeloeil::_, trompeloeil::_));
	// No expectation for "stack_1.mrcs": acquiring it would violate.

	executor_image_saver saver(
		writers,
		std::make_shared<synchronous_executor>()
	);
	const auto completion = saver.save(make_test_array(), plan, sanitizer);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"executor_image_saver resolves an empty plan without acquiring a writer",
	"[executor_image_saver]"
)
{
	const image_transaction_plan plan(
		image_transfer_shape(plane_extents, 3, 3)
	);

	// No expectations set on `writers` or on `sanitizer`: acquiring or
	// sanitizing anything would violate.
	const auto sanitizer = std::make_shared<mock_image_transfer_sanitizer>();
	const auto writers = std::make_shared<mock_image_writer_provider>();

	executor_image_saver saver(
		writers,
		std::make_shared<synchronous_executor>()
	);
	const auto completion = saver.save(make_test_array(), plan, sanitizer);

	REQUIRE( completion != nullptr );
	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"executor_image_saver's completion reports what a writer threw",
	"[executor_image_saver]"
)
{
	image_transaction_plan plan(image_transfer_shape(plane_extents, 3, 3));
	const auto zero = plan.add_file("stack_0.mrcs");
	add_element(plan, zero, 0, 0);

	const auto sanitizer = std::make_shared<mock_image_transfer_sanitizer>();
	const auto passing_through = allow_passing_through(*sanitizer);

	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("stack_0.mrcs")).RETURN(writer);
	ALLOW_CALL(*writer, get_descriptor()).RETURN(std::ref(stack_descriptor));
	REQUIRE_CALL(*writer, write(trompeloeil::_, trompeloeil::_))
		.SIDE_EFFECT( throw std::runtime_error("from a writer") );

	executor_image_saver saver(
		writers,
		std::make_shared<synchronous_executor>()
	);
	const auto completion = saver.save(make_test_array(), plan, sanitizer);

	REQUIRE( completion->is_ready() );
	REQUIRE_THROWS_AS( completion->get(), std::runtime_error );
}

TEST_CASE(
	"executor_image_saver shows each file's regions to the sanitizer beside "
	"the extents of both sides",
	"[executor_image_saver]"
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

	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer_zero = std::make_shared<mock_image_writer>();
	const auto writer_one = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("stack_0.mrcs")).RETURN(writer_zero);
	REQUIRE_CALL(*writers, acquire("stack_1.mrcs")).RETURN(writer_one);
	ALLOW_CALL(*writer_zero, get_descriptor())
		.RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(*writer_one, get_descriptor())
		.LR_RETURN(std::ref(short_stack_descriptor));
	REQUIRE_CALL(*writer_zero, write(trompeloeil::_, trompeloeil::_));
	REQUIRE_CALL(*writer_one, write(trompeloeil::_, trompeloeil::_));

	executor_image_saver saver(
		writers,
		std::make_shared<synchronous_executor>()
	);
	const auto completion = saver.save(make_test_array(), plan, sanitizer);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"executor_image_saver writes every plan the sanitizer answers with",
	"[executor_image_saver]"
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

	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("stack_0.mrcs")).RETURN(writer);
	ALLOW_CALL(*writer, get_descriptor()).RETURN(std::ref(stack_descriptor));
	REQUIRE_CALL(*writer, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH( _2.get_shape().get_extents()[0] == 2 );
	REQUIRE_CALL(*writer, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH( _2.get_shape().get_extents()[0] == 1 );

	executor_image_saver saver(
		writers,
		std::make_shared<synchronous_executor>()
	);
	const auto completion = saver.save(make_test_array(), plan, sanitizer);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"executor_image_saver writes nothing of a file the sanitizer answers no "
	"plan for",
	"[executor_image_saver]"
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

	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("stack_0.mrcs")).RETURN(writer);
	ALLOW_CALL(*writer, get_descriptor()).RETURN(std::ref(stack_descriptor));
	// No expectation for `write`: writing anything would violate.

	executor_image_saver saver(
		writers,
		std::make_shared<synchronous_executor>()
	);
	const auto completion = saver.save(make_test_array(), plan, sanitizer);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"executor_image_saver's completion reports what the sanitizer threw",
	"[executor_image_saver]"
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

	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("stack_0.mrcs")).RETURN(writer);
	ALLOW_CALL(*writer, get_descriptor()).RETURN(std::ref(stack_descriptor));
	// No expectation for `write`: writing anything would violate.

	executor_image_saver saver(
		writers,
		std::make_shared<synchronous_executor>()
	);
	const auto completion = saver.save(make_test_array(), plan, sanitizer);

	REQUIRE( completion->is_ready() );
	REQUIRE_THROWS_AS( completion->get(), std::out_of_range );
}

TEST_CASE(
	"executor_image_saver submits each file as a task of its own",
	"[executor_image_saver]"
)
{
	// Running the tasks, and how many at once, is the executor's business.
	// The saver's is to submit one task per file and wait for none of them.
	static REXLIB_CONST_CONSTEXPR std::size_t file_count = 4;

	image_transaction_plan plan(image_transfer_shape(plane_extents, 3, 3));
	for (std::size_t i = 0; i < file_count; ++i)
	{
		const auto file = plan.add_file("stack_" + std::to_string(i) + ".mrcs");
		add_element(plan, file, 0, i);
	}

	// No expectations set on `writers` or on `sanitizer`: writing is what
	// the tasks do.
	const auto sanitizer = std::make_shared<mock_image_transfer_sanitizer>();
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto executor = std::make_shared<mock_executor>();

	REQUIRE_CALL(*executor, submit(trompeloeil::_, trompeloeil::_))
		.WITH( _1 != nullptr && _2 != nullptr )
		.TIMES(file_count);

	executor_image_saver saver(writers, executor);
	const auto completion = saver.save(make_test_array(), plan, sanitizer);

	CHECK_FALSE( completion->is_ready() );
}
