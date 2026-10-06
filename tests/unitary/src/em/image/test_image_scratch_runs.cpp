// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <em/image/image_scratch_runs.hpp>

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE(
	"image_scratch_runs cut the slots into runs of one length",
	"[image_scratch_runs]"
)
{
	// Ten slots in runs of four: two whole runs and one of what is left.
	const image_scratch_runs runs(10, 4);

	SECTION( "a slot is in the run that spans it" )
	{
		CHECK( runs.get_run(0) == 0 );
		CHECK( runs.get_run(3) == 0 );
		CHECK( runs.get_run(4) == 1 );
		CHECK( runs.get_run(7) == 1 );
		CHECK( runs.get_run(8) == 2 );
		CHECK( runs.get_run(9) == 2 );
	}

	SECTION( "a run starts where the one before it ends" )
	{
		CHECK( runs.get_first_slot(0) == 0 );
		CHECK( runs.get_first_slot(1) == 4 );
		CHECK( runs.get_first_slot(2) == 8 );
	}

	SECTION( "the last run spans what is left" )
	{
		CHECK( runs.get_slot_count(0) == 4 );
		CHECK( runs.get_slot_count(1) == 4 );
		CHECK( runs.get_slot_count(2) == 2 );
	}
}

TEST_CASE(
	"image_scratch_runs put every slot in one run when a run is long enough",
	"[image_scratch_runs]"
)
{
	const auto run_length = GENERATE(
		std::size_t(5),
		std::size_t(8),
		std::numeric_limits<std::size_t>::max()
	);
	const image_scratch_runs runs(5, run_length);

	CHECK( runs.get_run(0) == 0 );
	CHECK( runs.get_run(4) == 0 );
	CHECK( runs.get_first_slot(0) == 0 );
	CHECK( runs.get_slot_count(0) == 5 );
}

TEST_CASE(
	"image_scratch_runs of one slot each put every slot in a run of its own",
	"[image_scratch_runs]"
)
{
	const image_scratch_runs runs(3, 1);

	CHECK( runs.get_run(2) == 2 );
	CHECK( runs.get_first_slot(2) == 2 );
	CHECK( runs.get_slot_count(2) == 1 );
}

TEST_CASE(
	"image_scratch_runs start absent and stay present once marked",
	"[image_scratch_runs]"
)
{
	image_scratch_runs runs(10, 4);

	REQUIRE_FALSE( runs.is_present(0) );
	REQUIRE_FALSE( runs.is_present(1) );
	REQUIRE_FALSE( runs.is_present(2) );

	runs.mark_present(1);

	SECTION( "the run marked is present" )
	{
		CHECK( runs.is_present(1) );
	}

	SECTION( "the others are still absent" )
	{
		CHECK_FALSE( runs.is_present(0) );
		CHECK_FALSE( runs.is_present(2) );
	}

	SECTION( "marking it again leaves it present" )
	{
		runs.mark_present(1);

		CHECK( runs.is_present(1) );
	}
}

TEST_CASE(
	"image_scratch_runs keep what is present when moved",
	"[image_scratch_runs]"
)
{
	image_scratch_runs runs(10, 4);
	runs.mark_present(2);

	const image_scratch_runs moved(std::move(runs));

	CHECK( moved.is_present(2) );
	CHECK_FALSE( moved.is_present(0) );
	CHECK( moved.get_slot_count(2) == 2 );
}

TEST_CASE(
	"image_scratch_runs refuse a run of no slot",
	"[image_scratch_runs]"
)
{
	REQUIRE_THROWS_AS( image_scratch_runs(10, 0), std::invalid_argument );
}
