// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <array>
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

} // anonymous namespace

TEST_CASE( "an image_transfer_plan starts empty of regions",
	"[image_transfer_plan]" )
{
	const std::vector<std::size_t> extents = {3, 5};
	const image_transfer_plan regions(image_transfer_shape(extents, 3, 3));

	REQUIRE( regions.get_region_count() == 0 );
	REQUIRE( to_vector(regions.get_shape().get_extents()) == extents );
	REQUIRE( regions.get_shape().get_rank() == 2 );
	REQUIRE( regions.get_shape().get_file_rank() == 3 );
	REQUIRE( regions.get_shape().get_array_rank() == 3 );
}

TEST_CASE( "an image_transfer_plan carries offsets on both sides",
	"[image_transfer_plan]" )
{
	const std::vector<std::size_t> extents = {3, 5};
	image_transfer_plan regions(image_transfer_shape(extents, 3, 3));

	SECTION( "reset states the shape shared by every region" )
	{
		REQUIRE( to_vector(regions.get_shape().get_extents()) == extents );
		REQUIRE( regions.get_shape().get_rank() == 2 );
		REQUIRE( regions.get_shape().get_file_rank() == 3 );
		REQUIRE( regions.get_shape().get_array_rank() == 3 );
		REQUIRE( regions.get_region_count() == 0 );
	}

	SECTION( "added regions are read back in order" )
	{
		const std::array<std::size_t, 3> first_file = {2, 0, 0};
		const std::array<std::size_t, 3> first_array = {0, 0, 0};
		const std::array<std::size_t, 3> second_file = {1, 0, 0};
		const std::array<std::size_t, 3> second_array = {1, 0, 0};
		regions.add(
			make_span(first_file),
			make_span(first_array)
		);
		regions.add(
			make_span(second_file),
			make_span(second_array)
		);

		REQUIRE( regions.get_region_count() == 2 );
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
		const std::array<std::size_t, 3> origin = {0, 0, 0};
		regions.add(make_span(origin), make_span(origin));
		regions.clear();

		REQUIRE( regions.get_region_count() == 0 );
		REQUIRE( to_vector(regions.get_shape().get_extents()) == extents );
		REQUIRE( regions.get_shape().get_file_rank() == 3 );
		REQUIRE( regions.get_shape().get_array_rank() == 3 );
	}
}

TEST_CASE( "an image_transfer_plan lets the two sides differ in rank",
	"[image_transfer_plan]" )
{
	SECTION( "a batch of patches cut from a two dimensional micrograph" )
	{
		const std::vector<std::size_t> extents = {4, 4};
		image_transfer_plan regions(image_transfer_shape(extents, 2, 3));

		const std::array<std::size_t, 2> file_offset = {10, 20};
		const std::array<std::size_t, 3> array_offset = {0, 0, 0};
		regions.add(
			make_span(file_offset),
			make_span(array_offset)
		);

		REQUIRE( regions.get_shape().get_rank() == 2 );
		REQUIRE( regions.get_shape().get_file_rank() == 2 );
		REQUIRE( regions.get_shape().get_array_rank() == 3 );
		REQUIRE( to_vector(regions.get_file_offset(0)) ==
			std::vector<std::size_t>{10, 20} );
	}

	SECTION( "a plane of a stack read into an array without that axis" )
	{
		// The side of higher rank may be either one, which the extents no
		// longer being tied to a side is what allows.
		const std::vector<std::size_t> extents = {3, 5};
		image_transfer_plan regions(image_transfer_shape(extents, 3, 2));

		const std::array<std::size_t, 3> file_offset = {2, 0, 0};
		const std::array<std::size_t, 2> array_offset = {0, 0};
		regions.add(
			make_span(file_offset),
			make_span(array_offset)
		);

		REQUIRE( regions.get_shape().get_file_rank() == 3 );
		REQUIRE( regions.get_shape().get_array_rank() == 2 );
	}
}

TEST_CASE( "an image_transfer_plan refuses an offset of the wrong rank",
	"[image_transfer_plan]" )
{
	const std::vector<std::size_t> extents = {3, 5};
	image_transfer_plan regions(image_transfer_shape(extents, 3, 3));

	const std::array<std::size_t, 2> two = {0, 0};
	const std::array<std::size_t, 3> three = {0, 0, 0};

	SECTION( "the file offset must have the file rank" )
	{
		REQUIRE_THROWS_AS(
			regions.add(make_span(two), make_span(three)),
			std::invalid_argument
		);
	}

	SECTION( "the array offset must have the array rank" )
	{
		REQUIRE_THROWS_AS(
			regions.add(make_span(three), make_span(two)),
			std::invalid_argument
		);
	}

	SECTION( "a refused region is not appended" )
	{
		REQUIRE_THROWS_AS(
			regions.add(make_span(two), make_span(three)),
			std::invalid_argument
		);
		REQUIRE( regions.get_region_count() == 0 );
	}
}

TEST_CASE( "an image_transfer_plan reused across calls stops allocating",
	"[image_transfer_plan]" )
{
	// This is the reason the regions are aggregated rather than passed one
	// by one: once the first fill has sized it, filling it again touches
	// the heap not at all.
	const std::vector<std::size_t> extents = {3, 5};
	const std::size_t count = 64;

	image_transfer_plan regions(image_transfer_shape(extents, 3, 3));
	regions.reserve(count);

	const auto fill = [&] ()
	{
		regions.clear();
		for (std::size_t i = 0; i < count; ++i)
		{
			const std::array<std::size_t, 3> file_offset = {i % 4, 0, 0};
			const std::array<std::size_t, 3> array_offset = {i, 0, 0};
			regions.add(
				make_span(file_offset),
				make_span(array_offset)
			);
		}
	};

	fill();
	const auto *file_data = regions.get_file_offset(0).data();
	const auto *array_data = regions.get_array_offset(0).data();

	SECTION( "refilling keeps every region" )
	{
		fill();

		REQUIRE( regions.get_region_count() == count );
		REQUIRE( to_vector(regions.get_array_offset(count - 1)) ==
			std::vector<std::size_t>{count - 1, 0, 0} );
	}

	SECTION( "refilling does not move the storage, so nothing is allocated" )
	{
		for (std::size_t repeat = 0; repeat < 8; ++repeat)
		{
			fill();
			REQUIRE( regions.get_file_offset(0).data() == file_data );
			REQUIRE( regions.get_array_offset(0).data() ==
				array_data );
		}
	}
}

TEST_CASE( "an image_transfer_plan has value semantics",
	"[image_transfer_plan]" )
{
	const std::vector<std::size_t> extents = {3, 5};
	image_transfer_plan regions(image_transfer_shape(extents, 3, 3));
	const std::array<std::size_t, 3> file_offset = {2, 0, 0};
	const std::array<std::size_t, 3> array_offset = {1, 0, 0};
	regions.add(
		make_span(file_offset),
		make_span(array_offset)
	);

	SECTION( "a copy holds the same regions" )
	{
		const image_transfer_plan copy(regions);

		REQUIRE( copy.get_region_count() == 1 );
		REQUIRE( to_vector(copy.get_shape().get_extents()) == extents );
		REQUIRE( to_vector(copy.get_file_offset(0)) ==
			std::vector<std::size_t>{2, 0, 0} );
	}

	SECTION( "a copy does not share storage with the plan it copies" )
	{
		image_transfer_plan copy(regions);
		copy.clear();

		REQUIRE( copy.get_region_count() == 0 );
		REQUIRE( regions.get_region_count() == 1 );
	}
}
