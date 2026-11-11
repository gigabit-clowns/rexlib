// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <backends/cpu/math/special/bessel_i0.hpp>

using namespace rexlib::cpu;
using Catch::Matchers::WithinRel;

TEST_CASE(
	"bessel_i0 should reproduce known values of I0",
	"[bessel_i0]"
)
{
	CHECK( bessel_i0(0.0) == 1.0 );
	CHECK_THAT( bessel_i0(0.5), WithinRel(1.0634833707413236, 1e-14) );
	CHECK_THAT( bessel_i0(1.0), WithinRel(1.2660658777520082, 1e-14) );
	CHECK_THAT( bessel_i0(2.0), WithinRel(2.279585302336067, 1e-14) );
	CHECK_THAT( bessel_i0(5.0), WithinRel(27.23987182360445, 1e-14) );
	CHECK_THAT( bessel_i0(10.0), WithinRel(2815.716628466255, 1e-13) );
	CHECK_THAT( bessel_i0(15.0), WithinRel(339649.3732979139, 1e-13) );
	CHECK_THAT( bessel_i0(20.0), WithinRel(43558282.55955353, 1e-13) );
}

TEST_CASE(
	"bessel_i0 should be even",
	"[bessel_i0]"
)
{
	CHECK( bessel_i0(-3.0) == bessel_i0(3.0) );
}
