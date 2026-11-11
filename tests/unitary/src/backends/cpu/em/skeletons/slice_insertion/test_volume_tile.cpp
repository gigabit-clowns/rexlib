// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/em/skeletons/slice_insertion/volume_tile.hpp>

using namespace rexlib::cpu;

namespace
{

using interval_list_type = volume_tile::interval_list_type;

} // anonymous namespace

TEST_CASE(
	"volume_tile should keep its stored ranges",
	"[volume_tile]"
)
{
	const volume_tile tile(8, 2, 6, 4, 8);

	CHECK( tile.get_extent() == 8 );
	CHECK( tile.get_z_begin() == 2 );
	CHECK( tile.get_z_end() == 6 );
	CHECK( tile.get_y_begin() == 4 );
	CHECK( tile.get_y_end() == 8 );
}

TEST_CASE(
	"list_stored_frequencies should map stored indices up to half the extent "
	"to themselves",
	"[volume_tile]"
)
{
	interval_list_type intervals;

	CHECK( list_stored_frequencies(8, 0, 3, intervals) == 1 );
	CHECK( intervals[0] == frequency_interval(0, 2) );

	CHECK( list_stored_frequencies(7, 1, 4, intervals) == 1 );
	CHECK( intervals[0] == frequency_interval(1, 3) );
}

TEST_CASE(
	"list_stored_frequencies should map stored indices beyond half the "
	"extent to negative frequencies",
	"[volume_tile]"
)
{
	interval_list_type intervals;

	CHECK( list_stored_frequencies(8, 5, 8, intervals) == 1 );
	CHECK( intervals[0] == frequency_interval(-3, -1) );

	CHECK( list_stored_frequencies(7, 4, 7, intervals) == 1 );
	CHECK( intervals[0] == frequency_interval(-3, -1) );
}

TEST_CASE(
	"list_stored_frequencies should give the index at half an even extent "
	"both its frequency and the negative one",
	"[volume_tile]"
)
{
	interval_list_type intervals;

	CHECK( list_stored_frequencies(8, 4, 5, intervals) == 2 );
	CHECK( intervals[0] == frequency_interval(-4, -4) );
	CHECK( intervals[1] == frequency_interval(4, 4) );

	CHECK( list_stored_frequencies(8, 2, 6, intervals) == 2 );
	CHECK( intervals[0] == frequency_interval(-4, -3) );
	CHECK( intervals[1] == frequency_interval(2, 4) );

	CHECK( list_stored_frequencies(8, 0, 8, intervals) == 2 );
	CHECK( intervals[0] == frequency_interval(-4, -1) );
	CHECK( intervals[1] == frequency_interval(0, 4) );
}

TEST_CASE(
	"list_stored_frequencies should keep the halves of an odd extent apart",
	"[volume_tile]"
)
{
	interval_list_type intervals;

	CHECK( list_stored_frequencies(7, 3, 5, intervals) == 2 );
	CHECK( intervals[0] == frequency_interval(-3, -3) );
	CHECK( intervals[1] == frequency_interval(3, 3) );

	CHECK( list_stored_frequencies(7, 0, 7, intervals) == 2 );
	CHECK( intervals[0] == frequency_interval(-3, -1) );
	CHECK( intervals[1] == frequency_interval(0, 3) );
}

TEST_CASE(
	"volume_tile should list the frequencies of its own ranges",
	"[volume_tile]"
)
{
	const volume_tile tile(8, 2, 6, 5, 8);
	interval_list_type intervals;

	CHECK( tile.list_z_frequencies(intervals) == 2 );
	CHECK( intervals[0] == frequency_interval(-4, -3) );
	CHECK( intervals[1] == frequency_interval(2, 4) );

	CHECK( tile.list_y_frequencies(intervals) == 1 );
	CHECK( intervals[0] == frequency_interval(-3, -1) );
}
