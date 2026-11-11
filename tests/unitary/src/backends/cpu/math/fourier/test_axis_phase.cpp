// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <backends/cpu/math/fourier/axis_phase.hpp>

#include <backends/cpu/math/scalar/pi.hpp>

#include <cmath>
#include <complex>
#include <vector>

using namespace rexlib::cpu;
using Catch::Matchers::WithinAbs;

TEST_CASE(
	"fill_axis_phase should rotate each frequency by the shift",
	"[axis_phase]"
)
{
	std::vector<std::complex<double>> phases(8);
	fill_axis_phase(phases.data(), 8, 8, 1.5);

	const double frequencies[] = { 0, 1, 2, 3, 4, -3, -2, -1 };
	for (std::size_t j = 0; j < 8; ++j)
	{
		const auto angle = 2.0 * pi * frequencies[j] * 1.5 / 8.0;
		CHECK_THAT( phases[j].real(), WithinAbs(std::cos(angle), 1e-15) );
		CHECK_THAT( phases[j].imag(), WithinAbs(std::sin(angle), 1e-15) );
	}
}

TEST_CASE(
	"fill_axis_phase should fill only the entries asked for",
	"[axis_phase]"
)
{
	std::vector<std::complex<float>> phases(5, std::complex<float>(7, 7));
	fill_axis_phase(phases.data(), 3, 8, 0.0);

	CHECK( phases[0] == std::complex<float>(1, 0) );
	CHECK( phases[2] == std::complex<float>(1, 0) );
	CHECK( phases[3] == std::complex<float>(7, 7) );
}

TEST_CASE(
	"fill_axis_phase should give signs for a shift of half an even extent",
	"[axis_phase]"
)
{
	std::vector<std::complex<double>> phases(6);
	fill_axis_phase(phases.data(), 6, 6, 3.0);

	for (std::size_t j = 0; j < 6; ++j)
	{
		const auto sign = j % 2 == 0 ? 1.0 : -1.0;
		CHECK_THAT( phases[j].real(), WithinAbs(sign, 1e-14) );
		CHECK_THAT( phases[j].imag(), WithinAbs(0.0, 1e-14) );
	}
}
