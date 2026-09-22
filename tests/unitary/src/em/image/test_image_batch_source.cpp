// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_batch_source.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/synchronous_executor.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_source.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include "../../core/hardware/mock/mock_buffer.hpp"
#include "mock/mock_image_reader.hpp"
#include "mock/mock_image_reader_provider.hpp"

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

// image_source clips every region to the file it comes from, so a mocked
// reader has to report extents an unclipped region fits in: the shape of the
// image the cases below read whole, and the stack the indexed one draws from.
const std::vector<std::size_t> image_extents = {3, 5};
const std::vector<std::size_t> stack_extents = {6, 4, 4};

array make_array(const std::vector<std::size_t> &extents)
{
	const auto storage = std::make_shared<mock_buffer>();
	const auto descriptor = make_contiguous_array_descriptor(
		make_span(extents),
		numerical_type::float32
	);
	return array(storage, descriptor);
}

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

std::shared_ptr<image_batch_source> make_batch_source(
	std::shared_ptr<image_reader_provider> readers
)
{
	auto source = std::make_shared<image_source>(
		std::move(readers),
		std::make_shared<synchronous_executor>()
	);
	return std::make_shared<image_batch_source>(std::move(source));
}

} // anonymous namespace

TEST_CASE(
	"image_batch_source needs a downstream source",
	"[image_batch_source]"
)
{
	REQUIRE_THROWS_AS(
		image_batch_source(nullptr),
		std::invalid_argument
	);
}

TEST_CASE(
	"image_batch_source validates the destination array",
	"[image_batch_source]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto batch = make_batch_source(readers);
	// No expectations set on `readers`: none of these calls may reach it.

	SECTION( "a destination with no extents" )
	{
		const std::vector<image_location> locations;

		REQUIRE_THROWS_AS(
			batch->read(make_array({}), make_span(locations)),
			std::invalid_argument
		);
	}

	SECTION( "a batch size that does not match the location count" )
	{
		const std::vector<image_location> locations = {
			image_location("a.mrc"),
			image_location("b.mrc")
		};

		REQUIRE_THROWS_AS(
			batch->read(make_array({3, 4, 4}), make_span(locations)),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"image_batch_source refuses to mix indexed and unindexed locations",
	"[image_batch_source]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto batch = make_batch_source(readers);
	// No expectations set on `readers`: a rejected batch must not reach it.

	SECTION( "an unindexed location following an indexed one" )
	{
		const std::vector<image_location> locations = {
			image_location("stack.mrcs", 0),
			image_location("plain.mrc")
		};

		REQUIRE_THROWS_AS(
			batch->read(make_array({2, 4, 4}), make_span(locations)),
			std::invalid_argument
		);
	}

	SECTION( "an indexed location following an unindexed one" )
	{
		const std::vector<image_location> locations = {
			image_location("plain.mrc"),
			image_location("stack.mrcs", 0)
		};

		REQUIRE_THROWS_AS(
			batch->read(make_array({2, 4, 4}), make_span(locations)),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"image_batch_source resolves an empty batch without touching the source",
	"[image_batch_source]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto batch = make_batch_source(readers);
	// No expectations set on `readers`: acquiring anything would violate.

	const std::vector<image_location> locations;
	const auto completion = batch->read(make_array({0, 4, 4}), make_span(locations));

	REQUIRE( completion != nullptr );
	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_batch_source addresses the whole file for unindexed locations",
	"[image_batch_source]"
)
{
	// No position_in_stack: every location names a file read as a whole
	// image, so the file rank matches the core rank and every file offset
	// stays at the origin.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader_a = std::make_shared<mock_image_reader>();
	const auto reader_b = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("a.mrc")).RETURN(reader_a);
	REQUIRE_CALL(*readers, acquire("b.mrc")).RETURN(reader_b);
	ALLOW_CALL(*reader_a, get_extents()).RETURN(make_span(image_extents));
	ALLOW_CALL(*reader_b, get_extents()).RETURN(make_span(image_extents));

	REQUIRE_CALL(*reader_a, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0}
		);
	REQUIRE_CALL(*reader_b, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{1, 0, 0}
		);

	const auto batch = make_batch_source(readers);
	const std::vector<image_location> locations = {
		image_location("a.mrc"),
		image_location("b.mrc")
	};

	const auto completion =
		batch->read(make_array({2, 3, 5}), make_span(locations));

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_batch_source groups repeated unindexed locations onto one file",
	"[image_batch_source]"
)
{
	// The same whole file read into two batch slots must be acquired once
	// and delivered as two regions of the same reader call.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("a.mrc")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(image_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 2 &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_array_offset(1)) ==
				std::vector<std::size_t>{1, 0, 0}
		);

	const auto batch = make_batch_source(readers);
	const std::vector<image_location> locations = {
		image_location("a.mrc"),
		image_location("a.mrc")
	};

	const auto completion =
		batch->read(make_array({2, 3, 5}), make_span(locations));

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_batch_source uses the stack position as the file offset",
	"[image_batch_source]"
)
{
	// Every location carries a position_in_stack, so the file rank grows
	// to match the array rank and that position becomes the leading file
	// offset while the array offset's leading axis tracks the batch slot.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("stack.mrcs")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(stack_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 3 &&
			_2.get_file_rank() == 3 &&
			_2.get_array_rank() == 3 &&
			to_vector(_2.get_extents()) == std::vector<std::size_t>{4, 4} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{2, 0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_file_offset(1)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_array_offset(1)) ==
				std::vector<std::size_t>{1, 0, 0} &&
			to_vector(_2.get_file_offset(2)) ==
				std::vector<std::size_t>{5, 0, 0} &&
			to_vector(_2.get_array_offset(2)) ==
				std::vector<std::size_t>{2, 0, 0}
		);

	const auto batch = make_batch_source(readers);
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 2),
		image_location("stack.mrcs", 0),
		image_location("stack.mrcs", 5)
	};

	const auto completion =
		batch->read(make_array({3, 4, 4}), make_span(locations));

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_batch_source reads a run of one stack as a single region",
	"[image_batch_source]"
)
{
	// Reading a stack a batch at a time: consecutive slots taking
	// consecutive positions are neighbours on both sides, so the batch is
	// one hyperrectangle rather than three.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("stack.mrcs")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(stack_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			to_vector(_2.get_extents()) ==
				std::vector<std::size_t>{3, 4, 4} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{2, 0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0}
		);

	const auto batch = make_batch_source(readers);
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 2),
		image_location("stack.mrcs", 3),
		image_location("stack.mrcs", 4)
	};

	const auto completion =
		batch->read(make_array({3, 4, 4}), make_span(locations));

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_batch_source's completion reports what the source threw",
	"[image_batch_source]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("a.mrc")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(image_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.SIDE_EFFECT( throw std::runtime_error("from a reader") );

	const auto batch = make_batch_source(readers);
	const std::vector<image_location> locations = { image_location("a.mrc") };

	const auto completion =
		batch->read(make_array({1, 3, 5}), make_span(locations));

	REQUIRE( completion->is_ready() );
	REQUIRE_THROWS_AS( completion->get(), std::runtime_error );
}
