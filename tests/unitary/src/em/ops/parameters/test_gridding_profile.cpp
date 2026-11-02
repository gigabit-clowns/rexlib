// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/ops/parameters/gridding_profile.hpp>

#include <sstream>
#include <string>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE(
	"to_string should name each gridding_profile",
	"[gridding_profile]"
)
{
	CHECK( std::string(to_string(gridding_profile::box)) == "box" );
	CHECK( std::string(to_string(gridding_profile::triangle)) == "triangle" );
	CHECK(
		std::string(to_string(gridding_profile::kaiser_bessel)) ==
		"kaiser_bessel"
	);
}

TEST_CASE(
	"operator<< should write the name of a gridding_profile",
	"[gridding_profile]"
)
{
	std::ostringstream oss;
	oss << gridding_profile::kaiser_bessel;

	CHECK( oss.str() == "kaiser_bessel" );
}
