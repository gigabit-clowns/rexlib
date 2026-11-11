// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/math/fourier/frequency_interval.hpp>

using namespace rexlib::cpu;

TEST_CASE(
	"frequency_interval should keep its ends and compare by them",
	"[frequency_interval]"
)
{
	const frequency_interval interval(-3, 4);

	CHECK( interval.get_lower() == -3 );
	CHECK( interval.get_upper() == 4 );
	CHECK( interval == frequency_interval(-3, 4) );
	CHECK( interval != frequency_interval(-3, 5) );
}

TEST_CASE(
	"frequency_interval should default to the frequency 0 alone",
	"[frequency_interval]"
)
{
	CHECK( frequency_interval() == frequency_interval(0, 0) );
}
