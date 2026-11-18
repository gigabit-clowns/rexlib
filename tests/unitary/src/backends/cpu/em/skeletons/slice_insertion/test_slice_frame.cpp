// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/em/skeletons/slice_insertion/slice_frame.hpp>

using namespace rexlib::cpu;

TEST_CASE(
	"slice_frame should scale the rows of the transform into frequency steps",
	"[slice_frame]"
)
{
	const slice_frame frame({{ 1, 0, 0 }}, {{ 0, 0.5, 0.5 }}, 2.0);

	CHECK( frame.get_x_step() == volume_frequency{{ 2, 0, 0 }} );
	CHECK( frame.get_y_step() == volume_frequency{{ 0, 1, 1 }} );
}

TEST_CASE(
	"slice_frame should map a slice frequency by its steps",
	"[slice_frame]"
)
{
	const slice_frame frame({{ 0, 1, 0 }}, {{ 0, 0, 1 }}, 1.0);

	CHECK( frame.map(3, -2) == volume_frequency{{ 0, 3, -2 }} );
}
