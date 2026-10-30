// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/em/ops/preprocessing/standardize_background_operation.hpp>

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

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
	"standardize_background_operation should keep the axes, radius and "
	"bounds it was given",
	"[standardize_background_operation]"
)
{
	const standardize_background_operation operation({ 2, 1 }, 12.5, -3, 4);

	const auto axes = operation.get_parameters().get_axes();
	CHECK(
		std::vector<std::size_t>(axes.begin(), axes.end()) ==
		std::vector<std::size_t>{ 2, 1 }
	);
	CHECK( operation.get_radius() == 12.5 );
	CHECK( operation.get_lower_bound() == -3 );
	CHECK( operation.get_upper_bound() == 4 );
}

TEST_CASE(
	"standardize_background_operation should accept infinite bounds and a "
	"radius of 0",
	"[standardize_background_operation]"
)
{
	const standardize_background_operation operation(
		{ 1 },
		0,
		-infinity,
		infinity
	);

	CHECK( operation.get_radius() == 0 );
	CHECK( operation.get_lower_bound() == -infinity );
	CHECK( operation.get_upper_bound() == infinity );
}

TEST_CASE(
	"standardize_background_operation should belong to the em component",
	"[standardize_background_operation]"
)
{
	const standardize_background_operation operation({ 1 }, 1, -1, 1);

	CHECK(
		std::string(operation.get_descriptor().get_component()) ==
		"rexlib.em"
	);
	CHECK(
		std::string(operation.get_descriptor().get_name()) ==
		"standardize_background"
	);
}

TEST_CASE(
	"standardize_background_operation should reject a negative or NaN "
	"radius",
	"[standardize_background_operation]"
)
{
	const auto radius = GENERATE(-1.0, not_a_number);

	CHECK_THROWS_AS(
		standardize_background_operation({ 1 }, radius, -1, 1),
		std::invalid_argument
	);
}

TEST_CASE(
	"standardize_background_operation should reject bounds that do not lie "
	"on either side of 0",
	"[standardize_background_operation]"
)
{
	const auto bounds = GENERATE(
		std::vector<double>{ 0, 1 },
		std::vector<double>{ -1, 0 },
		std::vector<double>{ 1, 2 },
		std::vector<double>{ -2, -1 },
		std::vector<double>{ 1, -1 },
		std::vector<double>{ not_a_number, 1 },
		std::vector<double>{ -1, not_a_number }
	);

	CHECK_THROWS_AS(
		standardize_background_operation({ 1 }, 1, bounds[0], bounds[1]),
		std::invalid_argument
	);
}

TEST_CASE(
	"standardize_background_operation should reject a repeated axis",
	"[standardize_background_operation]"
)
{
	CHECK_THROWS_AS(
		standardize_background_operation({ 1, 1 }, 1, -1, 1),
		std::invalid_argument
	);
}
