// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_location.hpp>

#include <limits>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
#include <algorithm>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE(
	"image_location holds a path and a zero based position",
	"[image_location]"
)
{
	SECTION( "a default constructed location addresses nothing" )
	{
		const image_location location;

		REQUIRE( location.get_path().empty() );
		REQUIRE( location.get_position_in_stack() ==
			image_location::no_position );
	}

	SECTION( "a location without a position addresses the whole file" )
	{
		const image_location location("stack.mrcs");

		REQUIRE( location.get_path() == "stack.mrcs" );
		REQUIRE( location.get_position_in_stack() ==
			image_location::no_position );
	}

	SECTION( "a location with a position addresses one element" )
	{
		const image_location location("stack.mrcs", 2);

		REQUIRE( location.get_path() == "stack.mrcs" );
		REQUIRE( location.get_position_in_stack() == 2 );
	}

	SECTION( "the sentinel is the largest representable position" )
	{
		REQUIRE( image_location::no_position ==
			std::numeric_limits<std::size_t>::max() );
	}
}

TEST_CASE( "image_location has value semantics", "[image_location]" )
{
	const image_location location("stack.mrcs", 7);

	SECTION( "copies compare equal to their source" )
	{
		const image_location copy(location);

		REQUIRE( copy == location );
		REQUIRE( copy.get_path() == "stack.mrcs" );
		REQUIRE( copy.get_position_in_stack() == 7 );
	}

	SECTION( "assignment replaces both components" )
	{
		image_location assigned("other.mrc");
		assigned = location;

		REQUIRE( assigned == location );
	}

	SECTION( "moves transfer both components" )
	{
		image_location source("stack.mrcs", 7);
		const image_location moved(std::move(source));

		REQUIRE( moved == location );
	}

	SECTION( "locations differing in position are not equal" )
	{
		REQUIRE( location != image_location("stack.mrcs", 8) );
		REQUIRE( location != image_location("stack.mrcs") );
	}

	SECTION( "locations differing in path are not equal" )
	{
		REQUIRE( location != image_location("other.mrcs", 7) );
	}
}

TEST_CASE( "image_location orders by path before position", "[image_location]" )
{
	SECTION( "the path dominates the comparison" )
	{
		const image_location first("a.mrcs", 100);
		const image_location second("b.mrcs", 0);

		REQUIRE( first < second );
		REQUIRE( second > first );
		REQUIRE_FALSE( second < first );
	}

	SECTION( "positions order within one path" )
	{
		const image_location first("a.mrcs", 0);
		const image_location second("a.mrcs", 1);

		REQUIRE( first < second );
		REQUIRE( first <= second );
		REQUIRE( second >= first );
	}

	SECTION( "equal locations are neither less nor greater" )
	{
		const image_location first("a.mrcs", 3);
		const image_location second("a.mrcs", 3);

		REQUIRE_FALSE( first < second );
		REQUIRE_FALSE( second < first );
		REQUIRE( first <= second );
		REQUIRE( first >= second );
	}

	SECTION( "sorting groups every element of a file together" )
	{
		std::vector<image_location> locations = {
			image_location("b.mrcs", 1),
			image_location("a.mrcs", 5),
			image_location("b.mrcs", 0),
			image_location("a.mrcs", 2),
		};
		std::sort(locations.begin(), locations.end());

		REQUIRE( locations[0] == image_location("a.mrcs", 2) );
		REQUIRE( locations[1] == image_location("a.mrcs", 5) );
		REQUIRE( locations[2] == image_location("b.mrcs", 0) );
		REQUIRE( locations[3] == image_location("b.mrcs", 1) );
	}
}

TEST_CASE( "image_location hashes consistently", "[image_location]" )
{
	SECTION( "equal locations hash equally" )
	{
		const image_location first("stack.mrcs", 4);
		const image_location second("stack.mrcs", 4);

		REQUIRE( first.hash() == second.hash() );
		REQUIRE( std::hash<image_location>()(first) == first.hash() );
	}

	SECTION( "the position takes part in the hash" )
	{
		const image_location first("stack.mrcs", 4);
		const image_location second("stack.mrcs", 5);

		REQUIRE( first.hash() != second.hash() );
	}

	SECTION( "locations are usable as keys of an unordered container" )
	{
		std::unordered_set<image_location> locations;
		locations.insert(image_location("stack.mrcs", 0));
		locations.insert(image_location("stack.mrcs", 1));
		locations.insert(image_location("stack.mrcs", 0));

		REQUIRE( locations.size() == 2 );
		REQUIRE( locations.count(image_location("stack.mrcs", 1)) == 1 );
	}
}

TEST_CASE( "image_location parses its string form", "[image_location]" )
{
	image_location location;

	SECTION( "an indexed path yields a zero based position" )
	{
		REQUIRE( parse_image_location("3@stack.mrcs", location) );
		REQUIRE( location.get_path() == "stack.mrcs" );
		REQUIRE( location.get_position_in_stack() == 2 );
	}

	SECTION( "the smallest valid index is one" )
	{
		REQUIRE( parse_image_location("1@stack.mrcs", location) );
		REQUIRE( location.get_position_in_stack() == 0 );
	}

	SECTION( "a bare path addresses the whole file" )
	{
		REQUIRE( parse_image_location("stack.mrcs", location) );
		REQUIRE( location.get_path() == "stack.mrcs" );
		REQUIRE( location.get_position_in_stack() ==
			image_location::no_position );
	}

	SECTION( "a path may itself contain directories" )
	{
		REQUIRE( parse_image_location("12@a/b/stack.mrcs", location) );
		REQUIRE( location.get_path() == "a/b/stack.mrcs" );
		REQUIRE( location.get_position_in_stack() == 11 );
	}

	SECTION( "leading zeros are accepted" )
	{
		REQUIRE( parse_image_location("000004@stack.mrcs", location) );
		REQUIRE( location.get_position_in_stack() == 3 );
	}

	SECTION( "a zero index is rejected because the form is one based" )
	{
		REQUIRE_FALSE( parse_image_location("0@stack.mrcs", location) );
	}

	SECTION( "a malformed representation is rejected" )
	{
		REQUIRE_FALSE( parse_image_location("", location) );
		REQUIRE_FALSE( parse_image_location("@stack.mrcs", location) );
		REQUIRE_FALSE( parse_image_location("x@stack.mrcs", location) );
		REQUIRE_FALSE( parse_image_location("1x@stack.mrcs", location) );
		REQUIRE_FALSE( parse_image_location("-1@stack.mrcs", location) );
		REQUIRE_FALSE( parse_image_location("3@", location) );
	}

	SECTION( "an index that does not fit a size is rejected" )
	{
		REQUIRE_FALSE( parse_image_location(
			"99999999999999999999999999@stack.mrcs",
			location
		) );
	}

	SECTION( "a rejected representation leaves the result untouched" )
	{
		const image_location original("kept.mrcs", 9);
		location = original;

		REQUIRE_FALSE( parse_image_location("0@stack.mrcs", location) );
		REQUIRE( location == original );
	}
}

TEST_CASE( "image_location writes its string form", "[image_location]" )
{
	SECTION( "an addressed element carries a one based index" )
	{
		REQUIRE( to_string(image_location("stack.mrcs", 2)) ==
			"3@stack.mrcs" );
	}

	SECTION( "a whole file is written as a bare path" )
	{
		REQUIRE( to_string(image_location("stack.mrcs")) == "stack.mrcs" );
	}

	SECTION( "the stream operator agrees with to_string" )
	{
		const image_location location("stack.mrcs", 2);
		std::ostringstream output;
		output << location;

		REQUIRE( output.str() == to_string(location) );
	}

	SECTION( "every valid representation survives a round trip" )
	{
		const std::vector<std::string> texts = {
			"stack.mrcs",
			"1@stack.mrcs",
			"3@stack.mrcs",
			"12@a/b/stack.mrcs",
		};

		for (const auto &text : texts)
		{
			image_location location;
			REQUIRE( parse_image_location(text, location) );
			REQUIRE( to_string(location) == text );
		}
	}
}
