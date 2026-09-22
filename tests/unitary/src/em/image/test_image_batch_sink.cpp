// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_batch_sink.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/synchronous_executor.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/const_array.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_sink.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

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

const_array make_const_array(const std::vector<std::size_t> &extents)
{
	const auto storage = std::make_shared<mock_buffer>();
	const auto descriptor = make_contiguous_array_descriptor(
		make_span(extents),
		numerical_type::float32
	);
	array source(storage, descriptor);
	return source.share_const();
}

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

std::shared_ptr<image_batch_sink> make_batch_sink(
	std::shared_ptr<image_writer_provider> writers
)
{
	auto sink = std::make_shared<image_sink>(
		std::move(writers),
		std::make_shared<synchronous_executor>()
	);
	return std::make_shared<image_batch_sink>(std::move(sink));
}

} // anonymous namespace

TEST_CASE(
	"image_batch_sink needs a downstream sink",
	"[image_batch_sink]"
)
{
	REQUIRE_THROWS_AS(
		image_batch_sink(nullptr),
		std::invalid_argument
	);
}

TEST_CASE(
	"image_batch_sink validates the source array",
	"[image_batch_sink]"
)
{
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto batch = make_batch_sink(writers);
	// No expectations set on `writers`: none of these calls may reach it.

	SECTION( "a source with no extents" )
	{
		const std::vector<image_location> locations;

		REQUIRE_THROWS_AS(
			batch->write(make_const_array({}), make_span(locations)),
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
			batch->write(make_const_array({3, 4, 4}), make_span(locations)),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"image_batch_sink refuses to mix indexed and unindexed locations",
	"[image_batch_sink]"
)
{
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto batch = make_batch_sink(writers);
	// No expectations set on `writers`: a rejected batch must not reach it.

	SECTION( "an unindexed location following an indexed one" )
	{
		const std::vector<image_location> locations = {
			image_location("stack.mrcs", 0),
			image_location("plain.mrc")
		};

		REQUIRE_THROWS_AS(
			batch->write(make_const_array({2, 4, 4}), make_span(locations)),
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
			batch->write(make_const_array({2, 4, 4}), make_span(locations)),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"image_batch_sink resolves an empty batch without touching the sink",
	"[image_batch_sink]"
)
{
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto batch = make_batch_sink(writers);
	// No expectations set on `writers`: acquiring anything would violate.

	const std::vector<image_location> locations;
	const auto completion =
		batch->write(make_const_array({0, 4, 4}), make_span(locations));

	REQUIRE( completion != nullptr );
	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_batch_sink addresses the whole file for unindexed locations",
	"[image_batch_sink]"
)
{
	// No position_in_stack: every location names a file written as a whole
	// image, so the file rank matches the core rank and every file offset
	// stays at the origin.
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer_a = std::make_shared<mock_image_writer>();
	const auto writer_b = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("a.mrc")).RETURN(writer_a);
	REQUIRE_CALL(*writers, acquire("b.mrc")).RETURN(writer_b);

	REQUIRE_CALL(*writer_a, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0}
		);
	REQUIRE_CALL(*writer_b, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{1, 0, 0}
		);

	const auto batch = make_batch_sink(writers);
	const std::vector<image_location> locations = {
		image_location("a.mrc"),
		image_location("b.mrc")
	};

	const auto completion =
		batch->write(make_const_array({2, 3, 5}), make_span(locations));

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_batch_sink writes a run of one stack as a single region",
	"[image_batch_sink]"
)
{
	// A stack written a batch at a time, which is the case this exists for:
	// consecutive slots landing on consecutive positions are neighbours on
	// both sides, so the whole batch is one hyperrectangle rather than three.
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("particles.mrcs")).RETURN(writer);
	REQUIRE_CALL(*writer, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			_2.get_file_rank() == 3 &&
			_2.get_array_rank() == 3 &&
			to_vector(_2.get_extents()) ==
				std::vector<std::size_t>{3, 4, 4} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{6, 0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0}
		);

	const auto batch = make_batch_sink(writers);
	const std::vector<image_location> locations = {
		image_location("particles.mrcs", 6),
		image_location("particles.mrcs", 7),
		image_location("particles.mrcs", 8)
	};

	const auto completion =
		batch->write(make_const_array({3, 4, 4}), make_span(locations));

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_batch_sink writes a batch that is not one run slot by slot",
	"[image_batch_sink]"
)
{
	// Neighbours in part is not enough: every region of a plan spans the
	// same number of slots, so the two pairs here cannot merge while the
	// gap between them stays, and the batch is written one slot at a time.
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("particles.mrcs")).RETURN(writer);
	REQUIRE_CALL(*writer, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 4 &&
			to_vector(_2.get_extents()) == std::vector<std::size_t>{4, 4} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_file_offset(1)) ==
				std::vector<std::size_t>{1, 0, 0} &&
			to_vector(_2.get_file_offset(2)) ==
				std::vector<std::size_t>{4, 0, 0} &&
			to_vector(_2.get_file_offset(3)) ==
				std::vector<std::size_t>{5, 0, 0}
		);

	const auto batch = make_batch_sink(writers);
	const std::vector<image_location> locations = {
		image_location("particles.mrcs", 0),
		image_location("particles.mrcs", 1),
		image_location("particles.mrcs", 4),
		image_location("particles.mrcs", 5)
	};

	const auto completion =
		batch->write(make_const_array({4, 4, 4}), make_span(locations));

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_batch_sink spreads one batch over the stacks it names",
	"[image_batch_sink]"
)
{
	// Nothing binds a batch to one stack: each file is acquired once and
	// gets the slots that named it.
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto first = std::make_shared<mock_image_writer>();
	const auto second = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("first.mrcs")).RETURN(first);
	REQUIRE_CALL(*writers, acquire("second.mrcs")).RETURN(second);

	REQUIRE_CALL(*first, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 2 &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_array_offset(1)) ==
				std::vector<std::size_t>{2, 0, 0}
		);
	REQUIRE_CALL(*second, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{1, 0, 0}
		);

	const auto batch = make_batch_sink(writers);
	const std::vector<image_location> locations = {
		image_location("first.mrcs", 0),
		image_location("second.mrcs", 0),
		image_location("first.mrcs", 1)
	};

	const auto completion =
		batch->write(make_const_array({3, 4, 4}), make_span(locations));

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_batch_sink's completion reports what the sink threw",
	"[image_batch_sink]"
)
{
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("a.mrc")).RETURN(writer);
	REQUIRE_CALL(*writer, write(trompeloeil::_, trompeloeil::_))
		.SIDE_EFFECT( throw std::out_of_range("past the end of the stack") );

	const auto batch = make_batch_sink(writers);
	const std::vector<image_location> locations = { image_location("a.mrc") };

	const auto completion =
		batch->write(make_const_array({1, 3, 5}), make_span(locations));

	REQUIRE( completion->is_ready() );
	REQUIRE_THROWS_AS( completion->get(), std::out_of_range );
}
