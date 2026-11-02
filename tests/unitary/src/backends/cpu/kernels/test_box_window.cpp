// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/kernels/box_window.hpp>

#include <array>

using namespace rexlib::cpu;

TEST_CASE(
	"box_window should place a sample on its nearest voxel with weight 1",
	"[box_window]"
)
{
	const box_window window;
	std::array<float, box_window::tap_count> weights{};

	CHECK( window.get_tap_count() == 1 );
	CHECK( window.place(2.3, weights.data()) == 2 );
	CHECK( weights[0] == 1.0f );
	CHECK( window.place(2.7, weights.data()) == 3 );
	CHECK( window.place(-2.3, weights.data()) == -2 );
}

TEST_CASE(
	"box_window should send a sample halfway between voxels away from 0",
	"[box_window]"
)
{
	const box_window window;
	std::array<double, box_window::tap_count> weights{};

	CHECK( window.place(2.5, weights.data()) == 3 );
	CHECK( window.place(-2.5, weights.data()) == -3 );
	CHECK( window.place(0.5, weights.data()) == 1 );
	CHECK( window.place(-0.5, weights.data()) == -1 );
}
