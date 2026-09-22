// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_patch_source.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/synchronous_executor.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_source.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/index_table.hpp>

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

// Big enough that a patch of ten by ten sits well inside it, so that a case
// only clips when it means to.
const std::vector<std::size_t> image_extents = {100, 100};
const std::vector<std::size_t> stack_extents = {6, 100, 100};
const std::vector<std::size_t> volume_extents = {60, 60, 60};

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

index_table make_positions(
	const std::vector<std::vector<std::size_t>> &positions,
	std::size_t rank
)
{
	index_table result(rank);
	for (const auto &position : positions)
	{
		result.add(make_span(position));
	}
	return result;
}

std::shared_ptr<image_patch_source> make_patch_source(
	std::shared_ptr<image_reader_provider> readers
)
{
	auto source = std::make_shared<image_source>(
		std::move(readers),
		std::make_shared<synchronous_executor>()
	);
	return std::make_shared<image_patch_source>(std::move(source));
}

} // anonymous namespace

TEST_CASE(
	"image_patch_source needs a downstream source",
	"[image_patch_source]"
)
{
	REQUIRE_THROWS_AS(
		image_patch_source(nullptr),
		std::invalid_argument
	);
}

TEST_CASE(
	"image_patch_source validates the destination against the positions",
	"[image_patch_source]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto patches = make_patch_source(readers);
	const image_location location("a.mrc");
	// No expectations set on `readers`: none of these calls may reach it.

	SECTION( "a destination with no extents" )
	{
		const auto positions = make_positions({}, 2);

		REQUIRE_THROWS_AS(
			patches->read(make_array({}), location, positions),
			std::invalid_argument
		);
	}

	SECTION( "a batch size that does not match the position count" )
	{
		const auto positions = make_positions({{10, 10}, {20, 20}}, 2);

		REQUIRE_THROWS_AS(
			patches->read(make_array({3, 10, 10}), location, positions),
			std::invalid_argument
		);
	}

	SECTION( "positions that do not have the rank of one patch" )
	{
		const auto positions = make_positions({{10, 10, 10}}, 3);

		REQUIRE_THROWS_AS(
			patches->read(make_array({1, 10, 10}), location, positions),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"image_patch_source resolves an empty batch without touching the source",
	"[image_patch_source]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto patches = make_patch_source(readers);
	// No expectations set on `readers`: acquiring anything would violate.

	const auto positions = make_positions({}, 2);
	const auto completion =
		patches->read(make_array({0, 10, 10}), image_location("a.mrc"),
			positions);

	REQUIRE( completion != nullptr );
	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_patch_source puts a position at the middle of its patch",
	"[image_patch_source]"
)
{
	// The corner of a patch is its position less half its extent, so a
	// patch of ten centred at fifty starts at forty-five, and one of nine
	// centred there starts at forty-six.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("a.mrc")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(image_extents));

	const auto patches = make_patch_source(readers);
	const auto positions = make_positions({{50, 50}}, 2);

	SECTION( "an even extent" )
	{
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH(
				to_vector(_2.get_extents()) ==
					std::vector<std::size_t>{10, 10} &&
				to_vector(_2.get_file_offset(0)) ==
					std::vector<std::size_t>{45, 45}
			);

		const auto completion =
			patches->read(make_array({1, 10, 10}), image_location("a.mrc"),
				positions);

		REQUIRE( completion->is_ready() );
		CHECK_NOTHROW( completion->get() );
	}

	SECTION( "an odd extent" )
	{
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH(
				to_vector(_2.get_extents()) ==
					std::vector<std::size_t>{9, 9} &&
				to_vector(_2.get_file_offset(0)) ==
					std::vector<std::size_t>{46, 46}
			);

		const auto completion =
			patches->read(make_array({1, 9, 9}), image_location("a.mrc"),
				positions);

		REQUIRE( completion->is_ready() );
		CHECK_NOTHROW( completion->get() );
	}
}

TEST_CASE(
	"image_patch_source gives each patch its own slot of the batch",
	"[image_patch_source]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("a.mrc")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(image_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 3 &&
			_2.get_file_rank() == 2 &&
			_2.get_array_rank() == 3 &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{15, 25} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_file_offset(1)) ==
				std::vector<std::size_t>{35, 45} &&
			to_vector(_2.get_array_offset(1)) ==
				std::vector<std::size_t>{1, 0, 0} &&
			to_vector(_2.get_file_offset(2)) ==
				std::vector<std::size_t>{55, 65} &&
			to_vector(_2.get_array_offset(2)) ==
				std::vector<std::size_t>{2, 0, 0}
		);

	const auto patches = make_patch_source(readers);
	const auto positions =
		make_positions({{20, 30}, {40, 50}, {60, 70}}, 2);

	const auto completion =
		patches->read(make_array({3, 10, 10}), image_location("a.mrc"),
			positions);

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_patch_source carries the part of a patch before the image in its "
	"array offset",
	"[image_patch_source]"
)
{
	// A patch of ten centred at three starts two rows before the image
	// begins. The region still names a whole patch, and the downstream
	// source is what shortens it.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("a.mrc")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(image_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			to_vector(_2.get_extents()) ==
				std::vector<std::size_t>{8, 10} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{0, 45} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 2, 0}
		);

	const auto patches = make_patch_source(readers);
	const auto positions = make_positions({{3, 50}}, 2);

	const auto completion =
		patches->read(make_array({1, 10, 10}), image_location("a.mrc"),
			positions);

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_patch_source shortens a patch running past the far edge",
	"[image_patch_source]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("a.mrc")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(image_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			to_vector(_2.get_extents()) ==
				std::vector<std::size_t>{7, 10} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{93, 45} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0}
		);

	const auto patches = make_patch_source(readers);
	const auto positions = make_positions({{98, 50}}, 2);

	const auto completion =
		patches->read(make_array({1, 10, 10}), image_location("a.mrc"),
			positions);

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_patch_source cuts the patches of a stack out of one slice",
	"[image_patch_source]"
)
{
	// A location carrying a position in a stack grows the file rank by the
	// axis the stack is indexed along, which every patch of the batch
	// shares since they all come from one image.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("stack.mrcs")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(stack_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 2 &&
			_2.get_file_rank() == 3 &&
			_2.get_array_rank() == 3 &&
			to_vector(_2.get_extents()) ==
				std::vector<std::size_t>{10, 10} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{4, 15, 25} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_file_offset(1)) ==
				std::vector<std::size_t>{4, 35, 45} &&
			to_vector(_2.get_array_offset(1)) ==
				std::vector<std::size_t>{1, 0, 0}
		);

	const auto patches = make_patch_source(readers);
	const auto positions = make_positions({{20, 30}, {40, 50}}, 2);

	const auto completion =
		patches->read(make_array({2, 10, 10}),
			image_location("stack.mrcs", 4), positions);

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_patch_source cuts boxes out of a volume the same way",
	"[image_patch_source]"
)
{
	// Nothing about the class is two dimensional: a subtomogram is a patch
	// of one more axis.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("tomogram.mrc")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(volume_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			_2.get_file_rank() == 3 &&
			_2.get_array_rank() == 4 &&
			to_vector(_2.get_extents()) ==
				std::vector<std::size_t>{8, 8, 8} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{16, 26, 36} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0, 0}
		);

	const auto patches = make_patch_source(readers);
	const auto positions = make_positions({{20, 30, 40}}, 3);

	const auto completion =
		patches->read(make_array({1, 8, 8, 8}),
			image_location("tomogram.mrc"), positions);

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"image_patch_source's completion reports what the source threw",
	"[image_patch_source]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("a.mrc")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(image_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.SIDE_EFFECT( throw std::runtime_error("from a reader") );

	const auto patches = make_patch_source(readers);
	const auto positions = make_positions({{50, 50}}, 2);

	const auto completion =
		patches->read(make_array({1, 10, 10}), image_location("a.mrc"),
			positions);

	REQUIRE( completion->is_ready() );
	REQUIRE_THROWS_AS( completion->get(), std::runtime_error );
}
