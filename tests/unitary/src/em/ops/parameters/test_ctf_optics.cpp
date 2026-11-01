// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <rexlib/em/ops/parameters/ctf_optics.hpp>

#include <rexlib/core/platform/constexpr.hpp>

#include <limits>
#include <stdexcept>

using namespace rexlib;
using namespace rexlib::em;
using Catch::Matchers::WithinRel;

namespace
{

REXLIB_CONST_CONSTEXPR double infinity =
	std::numeric_limits<double>::infinity();
REXLIB_CONST_CONSTEXPR double not_a_number =
	std::numeric_limits<double>::quiet_NaN();

} // anonymous namespace

TEST_CASE(
	"ctf_optics should keep the constants it was given",
	"[ctf_optics]"
)
{
	const ctf_optics optics(300, 2.7, 0.1);

	CHECK( optics.get_voltage() == 300 );
	CHECK( optics.get_spherical_aberration() == 2.7 );
	CHECK( optics.get_amplitude_contrast() == 0.1 );
}

TEST_CASE(
	"ctf_optics should compute the relativistic wavelength",
	"[ctf_optics]"
)
{
	CHECK_THAT(
		ctf_optics(300, 2.7, 0.1).get_wavelength(),
		WithinRel(0.019687615309, 1e-9)
	);
	CHECK_THAT(
		ctf_optics(200, 2.7, 0.1).get_wavelength(),
		WithinRel(0.025079493567, 1e-9)
	);
}

TEST_CASE(
	"ctf_optics should accept a corrected or negative spherical aberration "
	"and no amplitude contrast",
	"[ctf_optics]"
)
{
	const auto spherical_aberration = GENERATE(0.0, -0.01);

	CHECK_NOTHROW( ctf_optics(300, spherical_aberration, 0.0) );
}

TEST_CASE(
	"ctf_optics should reject a voltage that is not finite and positive",
	"[ctf_optics]"
)
{
	const auto voltage = GENERATE(0.0, -300.0, infinity, not_a_number);

	CHECK_THROWS_AS( ctf_optics(voltage, 2.7, 0.1), std::invalid_argument );
}

TEST_CASE(
	"ctf_optics should reject a spherical aberration that is not finite",
	"[ctf_optics]"
)
{
	const auto spherical_aberration = GENERATE(infinity, not_a_number);

	CHECK_THROWS_AS(
		ctf_optics(300, spherical_aberration, 0.1),
		std::invalid_argument
	);
}

TEST_CASE(
	"ctf_optics should reject an amplitude contrast outside [0, 1)",
	"[ctf_optics]"
)
{
	const auto amplitude_contrast = GENERATE(-0.1, 1.0, 1.5, not_a_number);

	CHECK_THROWS_AS(
		ctf_optics(300, 2.7, amplitude_contrast),
		std::invalid_argument
	);
}
