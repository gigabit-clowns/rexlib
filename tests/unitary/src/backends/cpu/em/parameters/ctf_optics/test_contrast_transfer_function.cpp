// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <backends/cpu/em/parameters/ctf_optics/contrast_transfer_function.hpp>


#include <cmath>

#include <boost/math/constants/constants.hpp>

using namespace rexlib;
using namespace rexlib::cpu;
using boost::math::double_constants::pi;
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
	const contrast_transfer_function<double> ctf(
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
	const contrast_transfer_function<double> ctf(
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
	const contrast_transfer_function<double> along(
		optics, 1.0, 200, 18000, 12000, 0, 0
	);
	const contrast_transfer_function<double> turned(
		optics, 1.0, 200, 18000, 12000, pi / 2, 0
	);

	const auto with_u = reference(optics, 1.0, 200, 18000, 0, 30);
	const auto with_v = reference(optics, 1.0, 200, 12000, 0, 30);
	CHECK_THAT( along(30, 0), WithinAbs(with_u, 1e-9) );
	CHECK_THAT( along(0, 30), WithinAbs(with_v, 1e-9) );
	CHECK_THAT( turned(30, 0), WithinAbs(with_v, 1e-9) );
	CHECK_THAT( turned(0, 30), WithinAbs(with_u, 1e-9) );
}

TEST_CASE(
	"contrast_transfer_function should default to 0 at every frequency",
	"[contrast_transfer_function]"
)
{
	const contrast_transfer_function<double> ctf;

	CHECK( ctf(0, 0) == 0.0 );
	CHECK( ctf(12, -5) == 0.0 );
}

TEST_CASE(
	"contrast_transfer_function in single precision should stay close to "
	"double precision up to the Nyquist frequency",
	"[contrast_transfer_function]"
)
{
	// A large defocus over small pixels, where the phase is largest.
	const em::ctf_optics optics(300, 2.7, 0.1);
	const contrast_transfer_function<double> exact(
		optics, 0.5, 512, 50000, 45000, 0.7, 0.3
	);
	const contrast_transfer_function<float> single(
		optics, 0.5, 512, 50000, 45000, 0.7, 0.3
	);

	for (int k_x = 0; k_x <= 256; k_x += 17)
	{
		for (int k_y = -256; k_y <= 256; k_y += 31)
		{
			CHECK_THAT(
				static_cast<double>(single(
					static_cast<float>(k_x), static_cast<float>(k_y)
				)),
				WithinAbs(exact(k_x, k_y), 2e-3)
			);
		}
	}
}
