// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <backends/cpu/em/gridding/kaiser_bessel_window.hpp>

#include <backends/cpu/numerical/bessel_i0.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;
using Catch::Matchers::WithinAbs;

TEST_CASE(
	"kaiser_bessel_window should evaluate the normalized window within its "
	"half-width and 0 from it on",
	"[kaiser_bessel_window]"
)
{
	const kaiser_bessel_window<double> window(2, 9);

	CHECK( window.get_half_width() == 2.0 );
	CHECK( window.get_tap_count() == 4 );
	CHECK_THAT( window.evaluate(0.0), WithinAbs(1.0, 1e-12) );

	const auto expected = bessel_i0(9.0 * std::sqrt(0.75)) / bessel_i0(9.0);
	CHECK_THAT( window.evaluate(1.0), WithinAbs(expected, 1e-6) );
	CHECK_THAT( window.evaluate(-1.0), WithinAbs(expected, 1e-6) );
	CHECK( window.evaluate(2.0) == 0.0 );
	CHECK( window.evaluate(-2.5) == 0.0 );

	// The window does not vanish at the half-width.
	CHECK_THAT(
		window.evaluate(2.0 - 1e-9),
		WithinAbs(1.0 / bessel_i0(9.0), 1e-6)
	);
}

TEST_CASE(
	"kaiser_bessel_window should place each sample on the voxels within its "
	"half-width",
	"[kaiser_bessel_window]"
)
{
	const kaiser_bessel_window<double> window(2, 9);
	const std::array<double, 2> coordinates = {{ 0.3, 3.0 }};
	std::array<std::int32_t, 2> first{};
	std::vector<double> weights(2 * window.get_tap_count());

	window.place(coordinates.data(), 2, first.data(), weights.data(), 2);

	CHECK( first[0] == -1 );
	for (std::size_t tap = 0; tap < window.get_tap_count(); ++tap)
	{
		const auto distance = static_cast<double>(tap) - 1.0 - 0.3;
		CHECK( weights[tap * 2] == window.evaluate(distance) );
	}

	// A sample on a voxel reaches one voxel fewer, the farthest getting 0.
	CHECK( first[1] == 2 );
	CHECK( weights[3 * 2 + 1] == 0.0 );
}

TEST_CASE(
	"kaiser_bessel_window with a shape of 0 should be a box of its half-width",
	"[kaiser_bessel_window]"
)
{
	const kaiser_bessel_window<float> window(1.5, 0);

	CHECK( window.get_tap_count() == 3 );
	CHECK( window.evaluate(1.0f) == 1.0f );
	CHECK( window.evaluate(1.5f) == 0.0f );
}

TEST_CASE(
	"kaiser_bessel_window should give 0 to a voxel at the half-width or "
	"beyond, across blocks",
	"[kaiser_bessel_window]"
)
{
	const kaiser_bessel_window<float> window(1.0, 5);
	// More samples than one block, each on a voxel: the voxels one step
	// away lie at the half-width.
	std::vector<float> coordinates(100);
	for (std::size_t i = 0; i < coordinates.size(); ++i)
	{
		coordinates[i] = static_cast<float>(i) - 50.0f;
	}
	std::vector<std::int32_t> first(coordinates.size());
	std::vector<float> weights(window.get_tap_count() * coordinates.size());

	window.place(
		coordinates.data(),
		static_cast<std::int32_t>(coordinates.size()),
		first.data(),
		weights.data(),
		coordinates.size()
	);

	CHECK( window.get_tap_count() == 2 );
	for (std::size_t i = 0; i < coordinates.size(); ++i)
	{
		CHECK( first[i] == static_cast<std::int32_t>(i) - 50 );
		CHECK( weights[i] == 1.0f );
		CHECK( weights[coordinates.size() + i] == 0.0f );
	}
}
