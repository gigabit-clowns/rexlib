// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <backends/cpu/kernels/kaiser_bessel_window.hpp>

#include <backends/cpu/kernels/bessel_i0.hpp>

#include <cmath>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;
using Catch::Matchers::WithinAbs;

TEST_CASE(
	"kaiser_bessel_window should evaluate the normalized window within its "
	"half-width and 0 beyond",
	"[kaiser_bessel_window]"
)
{
	const kaiser_bessel_window window(em::gridding_window::kaiser_bessel(2, 9));

	CHECK( window.get_half_width() == 2.0 );
	CHECK( window.get_tap_count() == 4 );
	CHECK_THAT( window.evaluate(0.0), WithinAbs(1.0, 1e-12) );

	const auto expected = bessel_i0(9.0 * std::sqrt(0.75)) / bessel_i0(9.0);
	CHECK_THAT( window.evaluate(1.0), WithinAbs(expected, 1e-6) );
	CHECK_THAT( window.evaluate(-1.0), WithinAbs(expected, 1e-6) );
	CHECK( window.evaluate(2.0) == 0.0 );
	CHECK( window.evaluate(-2.5) == 0.0 );
}

TEST_CASE(
	"kaiser_bessel_window should place a sample on the voxels within its "
	"half-width",
	"[kaiser_bessel_window]"
)
{
	const kaiser_bessel_window window(em::gridding_window::kaiser_bessel(2, 9));
	std::vector<double> weights(window.get_tap_count());

	CHECK( window.place(0.3, weights.data()) == -1 );
	for (std::size_t tap = 0; tap < weights.size(); ++tap)
	{
		const auto distance = static_cast<double>(tap) - 1.0 - 0.3;
		CHECK( weights[tap] == window.evaluate(distance) );
	}

	// A sample on a voxel reaches one voxel fewer, the farthest getting 0.
	CHECK( window.place(3.0, weights.data()) == 2 );
	CHECK( weights[3] == 0.0 );
}

TEST_CASE(
	"kaiser_bessel_window with a shape of 0 should be a box of its half-width",
	"[kaiser_bessel_window]"
)
{
	const kaiser_bessel_window window(
		em::gridding_window::kaiser_bessel(1.5, 0)
	);

	CHECK( window.get_tap_count() == 3 );
	CHECK( window.evaluate(1.0) == 1.0 );
	CHECK( window.evaluate(1.5) == 0.0 );
}
