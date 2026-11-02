// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <backends/cpu/kernels/triangle_window.hpp>

#include <array>
#include <cstdint>

using namespace rexlib::cpu;
using Catch::Matchers::WithinAbs;

TEST_CASE(
	"triangle_window should split each sample linearly between the voxels "
	"around it",
	"[triangle_window]"
)
{
	const triangle_window window;
	const std::array<double, 3> coordinates = {{ 2.25, -0.5, -3.0 }};
	std::array<std::int32_t, 3> first{};
	// Tap-major: the voxels below, then the voxels above.
	std::array<double, 6> weights{};

	window.place(coordinates.data(), 3, first.data(), weights.data(), 3);

	CHECK( window.get_tap_count() == 2 );
	CHECK( first == std::array<std::int32_t, 3>{{ 2, -1, -3 }} );
	CHECK_THAT( weights[0], WithinAbs(0.75, 1e-15) );
	CHECK_THAT( weights[3], WithinAbs(0.25, 1e-15) );
	CHECK_THAT( weights[1], WithinAbs(0.5, 1e-15) );
	CHECK_THAT( weights[4], WithinAbs(0.5, 1e-15) );
	CHECK( weights[2] == 1.0 );
	CHECK( weights[5] == 0.0 );
}
