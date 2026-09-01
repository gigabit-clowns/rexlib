// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_region_batch.hpp>

#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

} // namespace

TEST_CASE( "an image_region_batch starts empty", "[image_region_batch]" )
{
	const image_region_batch regions;

	REQUIRE( regions.get_size() == 0 );
	REQUIRE( regions.get_file_rank() == 0 );
	REQUIRE( regions.get_array_rank() == 0 );
	REQUIRE( regions.get_extents().empty() );
}

TEST_CASE( "an image_region_batch carries offsets on both sides",
	"[image_region_batch]" )
{
	const std::vector<std::size_t> extents = {1, 3, 5};
	image_region_batch regions;
	regions.reset(make_span(extents), 3);

	SECTION( "reset states the shape shared by every region" )
	{
		REQUIRE( to_vector(regions.get_extents()) == extents );
		REQUIRE( regions.get_array_rank() == 3 );
		REQUIRE( regions.get_file_rank() == 3 );
		REQUIRE( regions.get_size() == 0 );
	}

	SECTION( "added regions are read back in order" )
	{
		const std::size_t first_file[3] = {2, 0, 0};
		const std::size_t first_array[3] = {0, 0, 0};
		const std::size_t second_file[3] = {1, 0, 0};
		const std::size_t second_array[3] = {1, 0, 0};
		regions.add(make_span(first_file, 3), make_span(first_array, 3));
		regions.add(make_span(second_file, 3), make_span(second_array, 3));

		REQUIRE( regions.get_size() == 2 );
		REQUIRE( to_vector(regions.get_file_offset(0)) ==
			std::vector<std::size_t>{2, 0, 0} );
		REQUIRE( to_vector(regions.get_array_offset(0)) ==
			std::vector<std::size_t>{0, 0, 0} );
		REQUIRE( to_vector(regions.get_file_offset(1)) ==
			std::vector<std::size_t>{1, 0, 0} );
		REQUIRE( to_vector(regions.get_array_offset(1)) ==
			std::vector<std::size_t>{1, 0, 0} );
	}

	SECTION( "clear drops the regions but keeps the shape" )
	{
		const std::size_t origin[3] = {0, 0, 0};
		regions.add(make_span(origin, 3), make_span(origin, 3));
		regions.clear();

		REQUIRE( regions.get_size() == 0 );
		REQUIRE( to_vector(regions.get_extents()) == extents );
		REQUIRE( regions.get_file_rank() == 3 );
	}

	SECTION( "resetting again drops the regions" )
	{
		const std::size_t origin[3] = {0, 0, 0};
		regions.add(make_span(origin, 3), make_span(origin, 3));

		const std::vector<std::size_t> other = {1, 1, 2};
		regions.reset(make_span(other), 3);

		REQUIRE( regions.get_size() == 0 );
		REQUIRE( to_vector(regions.get_extents()) == other );
	}
}

TEST_CASE( "an image_region_batch allows a file of lower rank",
	"[image_region_batch]" )
{
	SECTION( "a batch of patches cut from a two dimensional micrograph" )
	{
		const std::vector<std::size_t> extents = {1, 4, 4};
		image_region_batch regions;
		regions.reset(make_span(extents), 2);

		const std::size_t file_offset[2] = {10, 20};
		const std::size_t array_offset[3] = {0, 0, 0};
		regions.add(make_span(file_offset, 2), make_span(array_offset, 3));

		REQUIRE( regions.get_file_rank() == 2 );
		REQUIRE( regions.get_array_rank() == 3 );
		REQUIRE( to_vector(regions.get_file_offset(0)) ==
			std::vector<std::size_t>{10, 20} );
	}

	SECTION( "the extents the file does not span must be one" )
	{
		const std::vector<std::size_t> extents = {2, 4, 4};
		image_region_batch regions;

		REQUIRE_THROWS_AS(
			regions.reset(make_span(extents), 2),
			std::invalid_argument
		);
	}

	SECTION( "a file rank above the rank of the extents is refused" )
	{
		const std::vector<std::size_t> extents = {4, 4};
		image_region_batch regions;

		REQUIRE_THROWS_AS(
			regions.reset(make_span(extents), 3),
			std::invalid_argument
		);
	}
}

TEST_CASE( "an image_region_batch refuses an offset of the wrong rank",
	"[image_region_batch]" )
{
	const std::vector<std::size_t> extents = {1, 3, 5};
	image_region_batch regions;
	regions.reset(make_span(extents), 3);

	const std::size_t two[2] = {0, 0};
	const std::size_t three[3] = {0, 0, 0};

	SECTION( "the file offset must have the file rank" )
	{
		REQUIRE_THROWS_AS(
			regions.add(make_span(two, 2), make_span(three, 3)),
			std::invalid_argument
		);
	}

	SECTION( "the array offset must have the rank of the extents" )
	{
		REQUIRE_THROWS_AS(
			regions.add(make_span(three, 3), make_span(two, 2)),
			std::invalid_argument
		);
	}

	SECTION( "a refused region is not appended" )
	{
		REQUIRE_THROWS_AS(
			regions.add(make_span(two, 2), make_span(three, 3)),
			std::invalid_argument
		);
		REQUIRE( regions.get_size() == 0 );
	}
}

TEST_CASE( "an image_region_batch reused across batches stops allocating",
	"[image_region_batch]" )
{
	// This is the reason the regions are aggregated rather than passed one
	// by one: once the first fill has sized it, filling it again touches
	// the heap not at all.
	const std::vector<std::size_t> extents = {1, 3, 5};
	const std::size_t batch = 64;

	image_region_batch regions;
	regions.reset(make_span(extents), 3);
	regions.reserve(batch);

	const auto fill = [&] ()
	{
		regions.clear();
		for (std::size_t i = 0; i < batch; ++i)
		{
			const std::size_t file_offset[3] = {i % 4, 0, 0};
			const std::size_t array_offset[3] = {i, 0, 0};
			regions.add(
				make_span(file_offset, 3),
				make_span(array_offset, 3)
			);
		}
	};

	fill();
	const auto *file_data = regions.get_file_offset(0).data();
	const auto *array_data = regions.get_array_offset(0).data();

	SECTION( "refilling keeps every region" )
	{
		fill();

		REQUIRE( regions.get_size() == batch );
		REQUIRE( to_vector(regions.get_array_offset(batch - 1)) ==
			std::vector<std::size_t>{batch - 1, 0, 0} );
	}

	SECTION( "refilling does not move the storage, so nothing is allocated" )
	{
		for (std::size_t repeat = 0; repeat < 8; ++repeat)
		{
			fill();
			REQUIRE( regions.get_file_offset(0).data() == file_data );
			REQUIRE( regions.get_array_offset(0).data() == array_data );
		}
	}
}

TEST_CASE( "an image_region_batch has value semantics", "[image_region_batch]" )
{
	const std::vector<std::size_t> extents = {1, 3, 5};
	image_region_batch regions;
	regions.reset(make_span(extents), 3);
	const std::size_t file_offset[3] = {2, 0, 0};
	const std::size_t array_offset[3] = {1, 0, 0};
	regions.add(make_span(file_offset, 3), make_span(array_offset, 3));

	SECTION( "a copy holds the same regions" )
	{
		const image_region_batch copy(regions);

		REQUIRE( copy.get_size() == 1 );
		REQUIRE( to_vector(copy.get_extents()) == extents );
		REQUIRE( to_vector(copy.get_file_offset(0)) ==
			std::vector<std::size_t>{2, 0, 0} );
	}

	SECTION( "a copy does not share storage with its source" )
	{
		image_region_batch copy(regions);
		copy.clear();

		REQUIRE( copy.get_size() == 0 );
		REQUIRE( regions.get_size() == 1 );
	}
}
