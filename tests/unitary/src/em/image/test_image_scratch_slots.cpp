// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/image_scratch_slots.hpp>

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

const std::size_t max_index = std::numeric_limits<std::size_t>::max();

} // anonymous namespace

TEST_CASE(
	"image_scratch_slots hold each index at its position among them",
	"[image_scratch_slots]"
)
{
	const image_scratch_slots slots({1, 4, 5});

	REQUIRE( slots.get_count() == 3 );
	CHECK( slots.get_index(0) == 1 );
	CHECK( slots.get_index(1) == 4 );
	CHECK( slots.get_index(2) == 5 );
}

TEST_CASE(
	"image_scratch_slots find the first index held that is not below an "
	"index",
	"[image_scratch_slots]"
)
{
	const image_scratch_slots slots({1, 4, 5});

	SECTION( "an index held is at the slot found" )
	{
		CHECK( slots.get_lower_bound(1) == 0 );
		CHECK( slots.get_lower_bound(4) == 1 );
		CHECK( slots.get_lower_bound(5) == 2 );
	}

	SECTION( "an index that is not held finds the next one that is" )
	{
		CHECK( slots.get_lower_bound(0) == 0 );
		CHECK( slots.get_lower_bound(2) == 1 );
		CHECK( slots.get_lower_bound(3) == 1 );
	}

	SECTION( "an index above every one held finds none" )
	{
		CHECK( slots.get_lower_bound(6) == slots.get_count() );
		CHECK( slots.get_lower_bound(max_index) == slots.get_count() );
	}
}

TEST_CASE(
	"image_scratch_slots count how many of a run of indices are held",
	"[image_scratch_slots]"
)
{
	const image_scratch_slots slots({1, 4, 5});

	SECTION( "every one of a run that is held throughout" )
	{
		CHECK( slots.count_held(1, 1) == 1 );
		CHECK( slots.count_held(4, 2) == 2 );
	}

	SECTION( "some of a run that is held in part" )
	{
		CHECK( slots.count_held(0, 6) == 3 );
		CHECK( slots.count_held(3, 2) == 1 );
		CHECK( slots.count_held(5, 4) == 1 );
	}

	SECTION( "none of a run nothing of which is held" )
	{
		CHECK( slots.count_held(2, 2) == 0 );
		CHECK( slots.count_held(6, 3) == 0 );
	}

	SECTION( "none of a run of no index" )
	{
		CHECK( slots.count_held(4, 0) == 0 );
	}

	SECTION( "a run reaching past the largest index there is" )
	{
		CHECK( slots.count_held(4, max_index) == 2 );
		CHECK( slots.count_held(max_index, max_index) == 0 );
	}
}

TEST_CASE(
	"image_scratch_slots hold consecutive indices at consecutive slots",
	"[image_scratch_slots]"
)
{
	const image_scratch_slots slots({2, 3, 4, 9});

	const auto first = slots.get_lower_bound(2);

	REQUIRE( slots.count_held(2, 3) == 3 );
	CHECK( slots.get_index(first) == 2 );
	CHECK( slots.get_index(first + 1) == 3 );
	CHECK( slots.get_index(first + 2) == 4 );
}

TEST_CASE(
	"image_scratch_slots of no index hold nothing",
	"[image_scratch_slots]"
)
{
	const image_scratch_slots slots((std::vector<std::size_t>()));

	CHECK( slots.get_count() == 0 );
	CHECK( slots.get_lower_bound(0) == 0 );
	CHECK( slots.count_held(0, 8) == 0 );
}

TEST_CASE(
	"image_scratch_slots refuse indices that are not strictly ascending",
	"[image_scratch_slots]"
)
{
	SECTION( "indices that descend" )
	{
		REQUIRE_THROWS_AS( image_scratch_slots({3, 1}), std::invalid_argument );
	}

	SECTION( "an index given twice" )
	{
		REQUIRE_THROWS_AS( image_scratch_slots({1, 1}), std::invalid_argument );
	}
}
