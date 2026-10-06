// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/image_location_grouping.hpp>

#include <rexlib/em/image/image_location.hpp>

#include <cstddef>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

image_location_grouping group(const std::vector<image_location> &locations)
{
	return image_location_grouping(make_span(locations));
}

} // anonymous namespace

TEST_CASE(
	"an image_location_grouping of no location names no file",
	"[image_location_grouping]"
)
{
	const auto grouping = group({});

	CHECK( grouping.get_file_count() == 0 );
}

TEST_CASE(
	"an image_location_grouping names each file once, in the order the "
	"locations first name it",
	"[image_location_grouping]"
)
{
	// A list drawn at random from three stacks.
	const auto grouping = group({
		image_location("stack_1.mrcs", 4),
		image_location("stack_0.mrcs", 2),
		image_location("stack_1.mrcs", 0),
		image_location("stack_2.mrcs", 7),
		image_location("stack_0.mrcs", 5)
	});

	REQUIRE( grouping.get_file_count() == 3 );
	CHECK( grouping.get_path(0) == "stack_1.mrcs" );
	CHECK( grouping.get_path(1) == "stack_0.mrcs" );
	CHECK( grouping.get_path(2) == "stack_2.mrcs" );
}

TEST_CASE(
	"an image_location_grouping gathers the indices the locations name of "
	"each file",
	"[image_location_grouping]"
)
{
	const auto grouping = group({
		image_location("stack_1.mrcs", 4),
		image_location("stack_0.mrcs", 2),
		image_location("stack_1.mrcs", 0),
		image_location("stack_1.mrcs", 4),
		image_location("stack_1.mrcs", 1)
	});

	REQUIRE( grouping.get_file_count() == 2 );

	SECTION( "ascending, whatever order the locations name them in" )
	{
		const std::vector<std::size_t> indices = {0, 1, 4};

		CHECK( to_vector(grouping.get_indices(0)) == indices );
	}

	SECTION( "each file with its own" )
	{
		const std::vector<std::size_t> indices = {2};

		CHECK( to_vector(grouping.get_indices(1)) == indices );
	}

	SECTION( "none of them as a whole" )
	{
		CHECK_FALSE( grouping.is_whole(0) );
		CHECK_FALSE( grouping.is_whole(1) );
	}
}

TEST_CASE(
	"an image_location_grouping tells the files named as a whole",
	"[image_location_grouping]"
)
{
	const auto grouping = group({
		image_location("volume.mrc"),
		image_location("stack_0.mrcs", 3),
		image_location("stack_1.mrcs", 6),
		image_location("stack_1.mrcs"),
		image_location("stack_1.mrcs", 2)
	});

	REQUIRE( grouping.get_file_count() == 3 );

	SECTION( "a file only ever named as a whole carries no index" )
	{
		CHECK( grouping.is_whole(0) );
		CHECK( grouping.get_indices(0).empty() );
	}

	SECTION( "a file only ever named by index is not whole" )
	{
		CHECK_FALSE( grouping.is_whole(1) );
	}

	SECTION( "a file named both ways is whole and keeps its indices" )
	{
		const std::vector<std::size_t> indices = {2, 6};

		CHECK( grouping.is_whole(2) );
		CHECK( to_vector(grouping.get_indices(2)) == indices );
	}
}

TEST_CASE(
	"an image_location_grouping tells files apart by their paths as "
	"written",
	"[image_location_grouping]"
)
{
	const auto grouping = group({
		image_location("stack.mrcs", 0),
		image_location("./stack.mrcs", 1)
	});

	REQUIRE( grouping.get_file_count() == 2 );
	CHECK( grouping.get_path(0) == "stack.mrcs" );
	CHECK( grouping.get_path(1) == "./stack.mrcs" );
}
