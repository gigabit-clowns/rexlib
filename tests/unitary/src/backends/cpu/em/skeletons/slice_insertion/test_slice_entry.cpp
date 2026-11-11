// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/em/skeletons/slice_insertion/slice_entry.hpp>

using namespace rexlib::cpu;

TEST_CASE(
	"slice_entry should keep what it was given",
	"[slice_entry]"
)
{
	const slice_frame frame({{ 1, 0, 0 }}, {{ 0, 1, 0 }}, 2.0);
	const slice_entry entry(frame, 3, 24, 5, 40);

	CHECK( entry.get_frame().get_x_step() == volume_frequency{{ 2, 0, 0 }} );
	CHECK( entry.get_last_shell() == 3 );
	CHECK( entry.get_slice_offset() == 24 );
	CHECK( entry.get_radial_offset() == 5 );
	CHECK( entry.get_phase_offset() == 40 );
}

TEST_CASE(
	"slice_entry should default to a slice without a shell to insert",
	"[slice_entry]"
)
{
	const slice_entry entry;

	CHECK( entry.get_last_shell() == -1 );
	CHECK( entry.get_slice_offset() == 0 );
}
