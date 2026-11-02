// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <backends/cpu/kernels/triangle_window.hpp>

#include <array>

using namespace rexlib::cpu;
using Catch::Matchers::WithinAbs;

TEST_CASE(
	"triangle_window should split a sample linearly between the voxels "
	"around it",
	"[triangle_window]"
)
{
	const triangle_window window;
	std::array<double, triangle_window::tap_count> weights{};

	CHECK( window.get_tap_count() == 2 );
	CHECK( window.place(2.25, weights.data()) == 2 );
	CHECK_THAT( weights[0], WithinAbs(0.75, 1e-15) );
	CHECK_THAT( weights[1], WithinAbs(0.25, 1e-15) );

	CHECK( window.place(-0.5, weights.data()) == -1 );
	CHECK_THAT( weights[0], WithinAbs(0.5, 1e-15) );
	CHECK_THAT( weights[1], WithinAbs(0.5, 1e-15) );
}

TEST_CASE(
	"triangle_window should give a sample on a voxel to that voxel alone",
	"[triangle_window]"
)
{
	const triangle_window window;
	std::array<float, triangle_window::tap_count> weights{};

	CHECK( window.place(-3.0, weights.data()) == -3 );
	CHECK( weights[0] == 1.0f );
	CHECK( weights[1] == 0.0f );
}
