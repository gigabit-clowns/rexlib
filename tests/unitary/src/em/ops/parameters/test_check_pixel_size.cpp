// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <em/ops/parameters/check_pixel_size.hpp>

#include <limits>
#include <stdexcept>
#include <string>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE(
	"check_pixel_size should return a finite and positive pixel size",
	"[check_pixel_size]"
)
{
	CHECK( check_pixel_size("ctf_2d", 1.25) == 1.25 );
}

TEST_CASE(
	"check_pixel_size should reject a pixel size that is not finite and "
	"positive, naming the operation",
	"[check_pixel_size]"
)
{
	const auto pixel_size = GENERATE(
		0.0,
		-1.0,
		std::numeric_limits<double>::infinity(),
		std::numeric_limits<double>::quiet_NaN()
	);

	try
	{
		check_pixel_size("ctf_2d", pixel_size);
		FAIL( "no exception was thrown" );
	}
	catch (const std::invalid_argument &error)
	{
		CHECK( std::string(error.what()).rfind("ctf_2d: ", 0) == 0 );
	}
}
