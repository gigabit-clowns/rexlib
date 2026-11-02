// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <backends/cpu/kernels/contrast_transfer_function.hpp>

#include <backends/cpu/kernels/pi.hpp>

#include <cmath>

using namespace rexlib;
using namespace rexlib::cpu;
using Catch::Matchers::WithinAbs;

namespace
{

/**
 * @brief The CTF without astigmatism, from the formula of the contract.
 */
double reference(
	const em::ctf_optics &optics,
	double pixel_size,
	std::size_t extent,
	double defocus,
	double phase_shift,
	double frequency
)
{
	const auto wavelength = optics.get_wavelength();
	const auto aberration = optics.get_spherical_aberration() * 1e7;
	const auto s = frequency / (static_cast<double>(extent) * pixel_size);
	const auto s2 = s * s;
	return std::sin(
		pi * wavelength * defocus * s2 -
		0.5 * pi * aberration * std::pow(wavelength, 3) * s2 * s2 +
		phase_shift + std::asin(optics.get_amplitude_contrast())
	);
}

} // anonymous namespace

TEST_CASE(
	"contrast_transfer_function should equal the amplitude contrast at the "
	"zero frequency",
	"[contrast_transfer_function]"
)
{
	const contrast_transfer_function ctf(
		em::ctf_optics(300, 2.7, 0.1), 1.0, 256, 15000, 12000, 0.3, 0
	);

	CHECK_THAT( ctf(0, 0), WithinAbs(0.1, 1e-15) );
}

TEST_CASE(
	"contrast_transfer_function should follow the formula without "
	"astigmatism",
	"[contrast_transfer_function]"
)
{
	const em::ctf_optics optics(300, 2.7, 0.07);
	const contrast_transfer_function ctf(
		optics, 1.2, 128, 20000, 20000, 1.1, 0.4
	);

	for (double frequency = 1.0; frequency <= 64.0; frequency += 7.0)
	{
		const auto expected =
			reference(optics, 1.2, 128, 20000, 0.4, frequency);
		CHECK_THAT( ctf(frequency, 0), WithinAbs(expected, 1e-9) );
		CHECK_THAT( ctf(0, frequency), WithinAbs(expected, 1e-9) );
		CHECK_THAT(
			ctf(frequency * 0.6, frequency * 0.8),
			WithinAbs(expected, 1e-9)
		);
	}
}

TEST_CASE(
	"contrast_transfer_function should apply the defocus along and across "
	"the astigmatism",
	"[contrast_transfer_function]"
)
{
	const em::ctf_optics optics(200, 2.0, 0.1);
	const contrast_transfer_function along(
		optics, 1.0, 200, 18000, 12000, 0, 0
	);
	const contrast_transfer_function turned(
		optics, 1.0, 200, 18000, 12000, pi / 2, 0
	);

	const auto with_u = reference(optics, 1.0, 200, 18000, 0, 30);
	const auto with_v = reference(optics, 1.0, 200, 12000, 0, 30);
	CHECK_THAT( along(30, 0), WithinAbs(with_u, 1e-9) );
	CHECK_THAT( along(0, 30), WithinAbs(with_v, 1e-9) );
	CHECK_THAT( turned(30, 0), WithinAbs(with_v, 1e-9) );
	CHECK_THAT( turned(0, 30), WithinAbs(with_u, 1e-9) );
}
