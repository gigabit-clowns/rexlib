// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/layout/index.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace rex;

TEST_CASE("sanitize_index with a in-bounds positive index should return the same value", "[index]")
{
	const std::ptrdiff_t extent = 16;
	const std::ptrdiff_t index = GENERATE(0, 4, 8, 15);

	const auto sanitized = sanitize_index(index, extent);
	REQUIRE( sanitized == index );
}

TEST_CASE("sanitize_index with a in-bounds negative index should return the poisitivized value", "[index]")
{
	const std::ptrdiff_t extent = 24;
	std::ptrdiff_t index;
	std::size_t expected;
	std::tie(index, expected) = GENERATE(
		table<std::ptrdiff_t, std::size_t>({
			{ -1, 23 },
			{ -4, 20 },
			{ -10, 14 },
			{ -24, 0 }
		})
	);

	const auto sanitized = sanitize_index(index, extent);
	REQUIRE( sanitized == expected );
}

TEST_CASE("sanitize_index with an out of bounds index should throw", "[index]")
{
	const std::ptrdiff_t extent = 12;
	std::ptrdiff_t index;
	std::string expected_error_msg;
	std::tie(index, expected_error_msg) = GENERATE(
		table<std::ptrdiff_t, std::string>({
			{ 12, "Index 12 is out of bounds for extent 12" },
			{ 13, "Index 13 is out of bounds for extent 12" },
			{ 128, "Index 128 is out of bounds for extent 12" },
			{ -13, "Reverse index -13 is out of bounds for extent 12" },
			{ -24, "Reverse index -24 is out of bounds for extent 12" },
			{ -200, "Reverse index -200 is out of bounds for extent 12" }
		})
	);

	REQUIRE_THROWS_MATCHES( 
		sanitize_index(index, extent),
		std::out_of_range,
		Catch::Matchers::Message(expected_error_msg)
	);
}
