// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/tiff/tiff_region_bounds.hpp>

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::tiff;

namespace
{

// A stack of three pages of four rows and five columns, against an array of
// two planes of the same size held contiguously.
const std::vector<std::size_t> file_extents = {3, 4, 5};
const std::vector<std::size_t> array_extents = {2, 4, 5};
const std::vector<std::ptrdiff_t> array_strides = {20, 5, 1};

image_descriptor describe_file()
{
	return image_descriptor(
		make_span(file_extents), 2, numerical_type::uint16);
}

image_transfer_plan one_region(
	const std::vector<std::size_t> &extents,
	const std::vector<std::size_t> &file_offset,
	const std::vector<std::size_t> &array_offset
)
{
	image_transfer_plan regions(image_transfer_shape(
		extents, file_offset.size(), array_offset.size()));
	regions.add(make_span(file_offset), make_span(array_offset));

	return regions;
}

void check(const image_transfer_plan &regions)
{
	check_region_bounds(
		regions,
		describe_file(),
		make_span(array_extents),
		make_span(array_strides),
		0
	);
}

} // anonymous namespace

TEST_CASE( "regions that fit the file and the array pass the bounds check",
	"[tiff_region_bounds]" )
{
	SECTION( "a page into a plane" )
	{
		REQUIRE_NOTHROW(
			check(one_region({4, 5}, {2, 0, 0}, {1, 0, 0})) );
	}

	SECTION( "a run of pages into the planes of the array" )
	{
		REQUIRE_NOTHROW(
			check(one_region({2, 4, 5}, {1, 0, 0}, {0, 0, 0})) );
	}

	SECTION( "a patch that reaches the last row and column of a page" )
	{
		REQUIRE_NOTHROW(
			check(one_region({2, 3}, {0, 2, 2}, {0, 0, 0})) );
	}

	SECTION( "no region at all" )
	{
		const image_transfer_plan regions(
			image_transfer_shape({4, 5}, 3, 3));

		REQUIRE_NOTHROW( check(regions) );
	}
}

TEST_CASE( "regions that do not fit the file are refused",
	"[tiff_region_bounds]" )
{
	SECTION( "a page the file does not have" )
	{
		REQUIRE_THROWS_AS(
			check(one_region({4, 5}, {3, 0, 0}, {0, 0, 0})),
			std::out_of_range
		);
	}

	SECTION( "a run of pages that reaches past the last one" )
	{
		REQUIRE_THROWS_AS(
			check(one_region({2, 4, 5}, {2, 0, 0}, {0, 0, 0})),
			std::out_of_range
		);
	}

	SECTION( "a region that reaches past the last row or column" )
	{
		REQUIRE_THROWS_AS(
			check(one_region({4, 5}, {0, 1, 0}, {0, 0, 0})),
			std::out_of_range
		);
		REQUIRE_THROWS_AS(
			check(one_region({4, 5}, {0, 0, 1}, {0, 0, 0})),
			std::out_of_range
		);
	}

	SECTION( "a region among good ones" )
	{
		auto regions = one_region({4, 5}, {0, 0, 0}, {0, 0, 0});
		regions.add(
			make_span(std::vector<std::size_t>{3, 0, 0}),
			make_span(std::vector<std::size_t>{1, 0, 0})
		);

		REQUIRE_THROWS_AS( check(regions), std::out_of_range );
	}
}

TEST_CASE( "regions that do not fit the array are refused",
	"[tiff_region_bounds]" )
{
	SECTION( "a plane the array does not have" )
	{
		REQUIRE_THROWS_AS(
			check(one_region({4, 5}, {0, 0, 0}, {2, 0, 0})),
			std::out_of_range
		);
	}

	SECTION( "a run of pages longer than the array" )
	{
		REQUIRE_THROWS_AS(
			check(one_region({3, 4, 5}, {0, 0, 0}, {0, 0, 0})),
			std::out_of_range
		);
	}
}

TEST_CASE( "regions of another rank than the file or the array are refused",
	"[tiff_region_bounds]" )
{
	SECTION( "stated against a file of another rank" )
	{
		REQUIRE_THROWS_AS(
			check(one_region({4, 5}, {0, 0}, {0, 0, 0})),
			std::invalid_argument
		);
	}

	SECTION( "stated against an array of another rank" )
	{
		REQUIRE_THROWS_AS(
			check(one_region({4, 5}, {0, 0, 0}, {0, 0})),
			std::invalid_argument
		);
	}

	SECTION( "strides that do not have the rank of the extents" )
	{
		const std::vector<std::ptrdiff_t> strides = {5, 1};

		REQUIRE_THROWS_AS(
			check_region_bounds(
				one_region({4, 5}, {0, 0, 0}, {0, 0, 0}),
				describe_file(),
				make_span(array_extents),
				make_span(strides),
				0
			),
			std::invalid_argument
		);
	}
}
