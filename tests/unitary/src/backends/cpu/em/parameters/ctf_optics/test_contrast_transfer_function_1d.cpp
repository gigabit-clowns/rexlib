// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <backends/cpu/em/parameters/ctf_optics/contrast_transfer_function_1d.hpp>

#include <cmath>

#include <boost/math/constants/constants.hpp>

using namespace rexlib;
using namespace rexlib::cpu;
using boost::math::double_constants::pi;
using Catch::Matchers::WithinAbs;

TEST_CASE(
	"contrast_transfer_function_1d should follow the formula",
	"[contrast_transfer_function_1d]"
)
{
	const em::ctf_optics optics(300, 2.7, 0.07);
	const contrast_transfer_function_1d<double> ctf(
		ctf_phase_coefficients(optics, 1.2, 128),
		20000,
		0.4
	);

	const auto wavelength = optics.get_wavelength();
	for (double k = 0.0; k <= 64.0; k += 7.0)
	{
		const auto s = k / (128 * 1.2);
		const auto expected = std::sin(
			pi * wavelength * 20000 * s * s -
			0.5 * pi * 2.7e7 * std::pow(wavelength, 3) * std::pow(s, 4) +
			0.4 + std::asin(0.07)
		);
		CHECK_THAT( ctf(k), WithinAbs(expected, 1e-9) );
	}
}

TEST_CASE(
	"contrast_transfer_function_1d should give its phase from the squared "
	"frequency",
	"[contrast_transfer_function_1d]"
)
{
	const contrast_transfer_function_1d<double> ctf(
		ctf_phase_coefficients(em::ctf_optics(200, 2.0, 0.1), 1.0, 64),
		15000,
		0
	);

	CHECK( ctf(5) == std::sin(ctf.compute_squared_phase(25)) );
	CHECK( ctf(-5) == ctf(5) );
}

TEST_CASE(
	"contrast_transfer_function_1d should default to 0 at every frequency",
	"[contrast_transfer_function_1d]"
)
{
	const contrast_transfer_function_1d<double> ctf;

	CHECK( ctf(0) == 0.0 );
	CHECK( ctf(12) == 0.0 );
}
