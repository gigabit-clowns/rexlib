// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <backends/cpu/em/skeletons/slice_insertion/tabulated_phase.hpp>

#include <backends/cpu/numerical/pi.hpp>

#include <cmath>
#include <complex>

using namespace rexlib::cpu;
using Catch::Matchers::WithinAbs;

TEST_CASE(
	"tabulated_phase should multiply by exp(-2 pi i g . C / N) with C half "
	"the extent rounded down",
	"[tabulated_phase]"
)
{
	const tabulated_phase<double> phase(7);
	const std::complex<double> value(1, 0);

	// Stored index 5 along y is the frequency -2; stored 2 along z is 2.
	const auto result = phase.apply(value, 3, 5, 2);
	const auto angle = -2.0 * pi * (3.0 - 2.0 + 2.0) * 3.0 / 7.0;

	CHECK_THAT( result.real(), WithinAbs(std::cos(angle), 1e-14) );
	CHECK_THAT( result.imag(), WithinAbs(std::sin(angle), 1e-14) );
}

TEST_CASE(
	"tabulated_phase should agree with the sign of an even extent",
	"[tabulated_phase]"
)
{
	const tabulated_phase<double> phase(8);
	const std::complex<double> value(2, -1);

	const auto odd = phase.apply(value, 1, 6, 0);
	const auto even = phase.apply(value, 1, 6, 1);
	CHECK_THAT( odd.real(), WithinAbs(-2.0, 1e-14) );
	CHECK_THAT( odd.imag(), WithinAbs(1.0, 1e-14) );
	CHECK_THAT( even.real(), WithinAbs(2.0, 1e-14) );
	CHECK_THAT( even.imag(), WithinAbs(-1.0, 1e-14) );
}
