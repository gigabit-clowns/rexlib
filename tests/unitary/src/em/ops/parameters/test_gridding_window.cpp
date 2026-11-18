// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/em/ops/parameters/gridding_window.hpp>

#include <rexlib/core/platform/constexpr.hpp>

#include <cmath>
#include <limits>
#include <stdexcept>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

REXLIB_CONST_CONSTEXPR double infinity =
	std::numeric_limits<double>::infinity();
REXLIB_CONST_CONSTEXPR double not_a_number =
	std::numeric_limits<double>::quiet_NaN();

} // anonymous namespace

TEST_CASE(
	"gridding_window::nearest should be a box of half-width 1/2",
	"[gridding_window]"
)
{
	const auto window = gridding_window::nearest();

	CHECK( window.get_profile() == gridding_profile::box );
	CHECK( window.get_half_width() == 0.5 );
	CHECK( window.get_shape() == 0.0 );
}

TEST_CASE(
	"gridding_window::linear should be a triangle of half-width 1",
	"[gridding_window]"
)
{
	const auto window = gridding_window::linear();

	CHECK( window.get_profile() == gridding_profile::triangle );
	CHECK( window.get_half_width() == 1.0 );
	CHECK( window.get_shape() == 0.0 );
}

TEST_CASE(
	"gridding_window::kaiser_bessel should keep its half-width and shape",
	"[gridding_window]"
)
{
	const auto window = gridding_window::kaiser_bessel(2.0, 9.5);

	CHECK( window.get_profile() == gridding_profile::kaiser_bessel );
	CHECK( window.get_half_width() == 2.0 );
	CHECK( window.get_shape() == 9.5 );
}

TEST_CASE(
	"gridding_window::kaiser_bessel should accept a shape of 0",
	"[gridding_window]"
)
{
	CHECK( gridding_window::kaiser_bessel(1.5, 0.0).get_shape() == 0.0 );
}

TEST_CASE(
	"gridding_window::kaiser_bessel should accept a shape of ln of the "
	"largest double",
	"[gridding_window]"
)
{
	const auto largest = std::log(std::numeric_limits<double>::max());
	const auto window = gridding_window::kaiser_bessel(1.5, largest);

	CHECK( window.get_shape() == largest );
}

TEST_CASE(
	"gridding_window::kaiser_bessel should reject a half-width that is not "
	"finite and positive",
	"[gridding_window]"
)
{
	const auto half_width = GENERATE(0.0, -1.0, infinity, not_a_number);

	CHECK_THROWS_AS(
		gridding_window::kaiser_bessel(half_width, 1.0),
		std::invalid_argument
	);
}

TEST_CASE(
	"gridding_window::kaiser_bessel should reject a shape that is not from 0 "
	"to ln of the largest double",
	"[gridding_window]"
)
{
	const auto shape = GENERATE(-1.0, 710.0, infinity, not_a_number);

	CHECK_THROWS_AS(
		gridding_window::kaiser_bessel(2.0, shape),
		std::invalid_argument
	);
}
