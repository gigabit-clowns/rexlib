// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_access_traits.hpp>

#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE( "image_access_flags compose as a flagset", "[image_access_traits]" )
{
	SECTION( "an empty flagset holds nothing" )
	{
		const image_access_flags flags;

		REQUIRE_FALSE( flags.contains(
			image_access_flag_bits::ordered_offsets) );
		REQUIRE_FALSE( flags.contains(
			image_access_flag_bits::concurrent_read) );
		REQUIRE_FALSE( flags.contains(
			image_access_flag_bits::memory_resident) );
	}

	SECTION( "bits combine and are recovered independently" )
	{
		const image_access_flags flags({
			image_access_flag_bits::ordered_offsets,
			image_access_flag_bits::memory_resident
		});

		REQUIRE( flags.contains(
			image_access_flag_bits::ordered_offsets) );
		REQUIRE( flags.contains(
			image_access_flag_bits::memory_resident) );
		REQUIRE_FALSE( flags.contains(
			image_access_flag_bits::concurrent_read) );
		REQUIRE( flags.count() == 2 );
	}

	SECTION( "the bits do not overlap" )
	{
		const image_access_flags all({
			image_access_flag_bits::ordered_offsets,
			image_access_flag_bits::concurrent_read,
			image_access_flag_bits::memory_resident
		});

		REQUIRE( all.count() == 3 );
	}

	SECTION( "every bit has a name" )
	{
		REQUIRE( std::string(to_string(
			image_access_flag_bits::ordered_offsets)) ==
			"ordered_offsets" );
		REQUIRE( std::string(to_string(
			image_access_flag_bits::concurrent_read)) ==
			"concurrent_read" );
		REQUIRE( std::string(to_string(
			image_access_flag_bits::memory_resident)) ==
			"memory_resident" );
	}

	SECTION( "a bit streams as its name" )
	{
		std::ostringstream output;
		output << image_access_flag_bits::concurrent_read;

		REQUIRE( output.str() == "concurrent_read" );
	}
}

TEST_CASE( "image_access_traits reports what a read costs",
	"[image_access_traits]" )
{
	SECTION( "default constructed traits describe nothing" )
	{
		const image_access_traits traits;
		std::vector<std::size_t> granularity;
		traits.get_granularity(granularity);

		REQUIRE( granularity.empty() );
		REQUIRE( traits.get_flags().count() == 0 );
		REQUIRE( traits.get_preferred_data_type() ==
			numerical_type::unknown );
	}

	SECTION( "an element wise format reports a granularity of ones" )
	{
		const std::vector<std::size_t> ones = {1, 1, 1};
		const image_access_traits traits(
			make_span(ones),
			image_access_flags({
				image_access_flag_bits::ordered_offsets,
				image_access_flag_bits::concurrent_read
			}),
			numerical_type::float32
		);

		std::vector<std::size_t> granularity;
		traits.get_granularity(granularity);

		REQUIRE( granularity == ones );
		REQUIRE( traits.get_flags().contains(
			image_access_flag_bits::concurrent_read) );
		REQUIRE( traits.get_preferred_data_type() ==
			numerical_type::float32 );
	}

	SECTION( "a plane wise codec reports the full trailing extents" )
	{
		const std::vector<std::size_t> plane = {1, 4096, 4096};
		const image_access_traits traits(
			make_span(plane),
			image_access_flags(),
			numerical_type::uint8
		);

		std::vector<std::size_t> granularity;
		traits.get_granularity(granularity);

		REQUIRE( granularity == plane );
		REQUIRE( traits.get_flags().count() == 0 );
	}

	SECTION( "the output parameter is cleared before being populated" )
	{
		const std::vector<std::size_t> ones = {1, 1};
		const image_access_traits traits(
			make_span(ones),
			image_access_flags(),
			numerical_type::int16
		);

		std::vector<std::size_t> granularity = {9, 9, 9, 9};
		traits.get_granularity(granularity);

		REQUIRE( granularity == ones );
	}

	SECTION( "traits copy and assign as values" )
	{
		const std::vector<std::size_t> ones = {1, 1, 1};
		const image_access_traits traits(
			make_span(ones),
			image_access_flags(image_access_flag_bits::memory_resident),
			numerical_type::float64
		);

		image_access_traits copy;
		copy = traits;

		std::vector<std::size_t> granularity;
		copy.get_granularity(granularity);

		REQUIRE( granularity == ones );
		REQUIRE( copy.get_flags().contains(
			image_access_flag_bits::memory_resident) );
		REQUIRE( copy.get_preferred_data_type() == numerical_type::float64 );
	}
}
