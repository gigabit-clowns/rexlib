// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_open_options.hpp>
#include <rexlib/em/image/image_metadata.hpp>

#include <sstream>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE( "image_open_options defaults to the first image",
	"[image_open_options]" )
{
	SECTION( "a default instance selects image zero with no hint" )
	{
		const image_open_options options;

		REQUIRE( options.get_image_index() == 0 );
		REQUIRE( options.get_access_hint() == image_access_hint::none );
	}

	SECTION( "the image index is settable" )
	{
		image_open_options options;
		options.set_image_index(4);

		REQUIRE( options.get_image_index() == 4 );
	}

	SECTION( "the access hint is settable" )
	{
		image_open_options options;
		options.set_access_hint(image_access_hint::random);

		REQUIRE( options.get_access_hint() == image_access_hint::random );
	}

	SECTION( "options copy as values" )
	{
		image_open_options options;
		options.set_image_index(2);
		options.set_access_hint(image_access_hint::sequential);

		const image_open_options copy(options);

		REQUIRE( copy.get_image_index() == 2 );
		REQUIRE( copy.get_access_hint() == image_access_hint::sequential );
	}

	SECTION( "every hint has a name" )
	{
		REQUIRE( std::string(to_string(image_access_hint::none)) == "none" );
		REQUIRE( std::string(to_string(image_access_hint::sequential)) ==
			"sequential" );
		REQUIRE( std::string(to_string(image_access_hint::random)) ==
			"random" );
	}

	SECTION( "a hint streams as its name" )
	{
		std::ostringstream output;
		output << image_access_hint::sequential;

		REQUIRE( output.str() == "sequential" );
	}
}

TEST_CASE( "image_metadata distinguishes unstated from zero",
	"[image_metadata]" )
{
	SECTION( "a default instance states nothing" )
	{
		const image_metadata metadata;
		std::vector<double> sampling;
		std::vector<double> origin;
		metadata.get_sampling(sampling);
		metadata.get_origin(origin);

		REQUIRE( sampling.empty() );
		REQUIRE( origin.empty() );
	}

	SECTION( "a stated sampling is returned per axis" )
	{
		const std::vector<double> expected = {1.0, 1.34, 1.34};
		const image_metadata metadata(expected, {});

		std::vector<double> sampling;
		std::vector<double> origin;
		metadata.get_sampling(sampling);
		metadata.get_origin(origin);

		REQUIRE( sampling == expected );
		REQUIRE( origin.empty() );
	}

	SECTION( "a sampling of one is not the same as an unstated sampling" )
	{
		const std::vector<double> ones = {1.0, 1.0};
		const image_metadata stated(ones, {});
		const image_metadata unstated;

		std::vector<double> from_stated;
		std::vector<double> from_unstated;
		stated.get_sampling(from_stated);
		unstated.get_sampling(from_unstated);

		REQUIRE( from_stated == ones );
		REQUIRE( from_unstated.empty() );
	}

	SECTION( "the output parameters are cleared before being populated" )
	{
		const std::vector<double> expected = {2.0};
		const image_metadata metadata(expected, expected);

		std::vector<double> sampling = {7.0, 7.0, 7.0};
		std::vector<double> origin = {7.0, 7.0, 7.0};
		metadata.get_sampling(sampling);
		metadata.get_origin(origin);

		REQUIRE( sampling == expected );
		REQUIRE( origin == expected );
	}
}
