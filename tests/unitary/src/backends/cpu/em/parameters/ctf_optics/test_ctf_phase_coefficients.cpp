// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <backends/cpu/em/parameters/ctf_optics/ctf_phase_coefficients.hpp>

#include <cmath>

#include <boost/math/constants/constants.hpp>

using namespace rexlib;
using namespace rexlib::cpu;
using boost::math::double_constants::pi;
using Catch::Matchers::WithinRel;

TEST_CASE(
	"ctf_phase_coefficients should derive the factors of the defocus, of the "
	"aberration and of the amplitude contrast",
	"[ctf_phase_coefficients]"
)
{
	const em::ctf_optics optics(300, 2.7, 0.07);
	const ctf_phase_coefficients coefficients(optics, 1.25, 200);

	const auto wavelength = optics.get_wavelength();
	const auto scale = 1.0 / (200 * 1.25);
	CHECK_THAT(
		coefficients.get_defocus_scale(),
		WithinRel(pi * wavelength * scale * scale, 1e-14)
	);
	CHECK_THAT(
		coefficients.get_aberration(),
		WithinRel(
			0.5 * pi * 2.7e7 * std::pow(wavelength, 3) * std::pow(scale, 4),
			1e-14
		)
	);
	CHECK_THAT(
		coefficients.get_amplitude_phase(),
		WithinRel(std::asin(0.07), 1e-15)
	);
}
