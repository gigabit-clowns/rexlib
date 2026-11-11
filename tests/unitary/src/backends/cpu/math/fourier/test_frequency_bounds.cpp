// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/math/fourier/frequency_bounds.hpp>

using namespace rexlib::cpu;

TEST_CASE(
	"frequency_bounds should keep the bounds of each axis",
	"[frequency_bounds]"
)
{
	const frequency_bounds bounds({{ 0, -4, -2 }}, {{ 8, 3, 5 }});

	CHECK( bounds.get_lower(0) == 0 );
	CHECK( bounds.get_upper(0) == 8 );
	CHECK( bounds.get_lower(1) == -4 );
	CHECK( bounds.get_upper(2) == 5 );
}

TEST_CASE(
	"frequency_bounds::widened should move every bound outwards",
	"[frequency_bounds]"
)
{
	const auto widened =
		frequency_bounds({{ 0, -4, -2 }}, {{ 8, 3, 5 }}).widened(1.5);

	CHECK( widened.get_lower(0) == -1.5 );
	CHECK( widened.get_upper(0) == 9.5 );
	CHECK( widened.get_lower(2) == -3.5 );
	CHECK( widened.get_upper(1) == 4.5 );
}
