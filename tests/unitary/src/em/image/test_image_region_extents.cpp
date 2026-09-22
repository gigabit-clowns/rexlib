// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <em/image/image_region_extents.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

const char *const caller = "a_calling_plan";

const std::vector<std::size_t> extents = {3, 5};

std::vector<std::size_t> sanitize(
	std::size_t file_rank,
	std::size_t array_rank
)
{
	return sanitize_region_extents(
		make_span(extents),
		file_rank,
		array_rank,
		caller
	);
}

} // anonymous namespace

TEST_CASE(
	"sanitize_region_extents copies extents that fit both sides",
	"[image_region_extents]"
)
{
	SECTION( "a side of higher rank, which spans one position along the "
		"axes they do not reach" )
	{
		CHECK( sanitize(3, 4) == extents );
	}

	SECTION( "sides of exactly their rank" )
	{
		CHECK( sanitize(2, 2) == extents );
	}

	SECTION( "no extents at all, which outrank nothing" )
	{
		const std::vector<std::size_t> none;

		CHECK( sanitize_region_extents(
			make_span(none), 0, 0, caller).empty() );
	}
}

TEST_CASE(
	"sanitize_region_extents refuses extents outranking a side",
	"[image_region_extents]"
)
{
	SECTION( "the file side" )
	{
		REQUIRE_THROWS_AS( sanitize(1, 3), std::invalid_argument );
	}

	SECTION( "the array side" )
	{
		REQUIRE_THROWS_AS( sanitize(3, 1), std::invalid_argument );
	}
}

TEST_CASE(
	"sanitize_region_extents names its caller and the side that did not fit",
	"[image_region_extents]"
)
{
	// The caller is what a reader of the message knows they constructed;
	// this is shared, and naming itself would tell them nothing.
	SECTION( "the file side" )
	{
		REQUIRE_THROWS_MATCHES(
			sanitize(1, 3),
			std::invalid_argument,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith(std::string(caller) + ": ") &&
				Catch::Matchers::ContainsSubstring("file")
			)
		);
	}

	SECTION( "the array side" )
	{
		REQUIRE_THROWS_MATCHES(
			sanitize(3, 1),
			std::invalid_argument,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith(std::string(caller) + ": ") &&
				Catch::Matchers::ContainsSubstring("array")
			)
		);
	}
}
