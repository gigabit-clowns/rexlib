// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/hardware/device_index.hpp>

#include <sstream>


using namespace rexlib;

TEST_CASE( "construct device_index should produce the expected value", "[device_index]" )
{
	device_index index("example-backend", 4);
	REQUIRE( index.get_backend_name() == "example-backend" );
	REQUIRE( index.get_device_id() == 4 );
}

TEST_CASE( "parsing a valid device_index string should produce the expected object", "[device_index]" )
{
	device_index index;

	REQUIRE( parse_device_index("cuda:3", index) );
	REQUIRE( index.get_backend_name() == "cuda" );
	REQUIRE( index.get_device_id() == 3 );

	REQUIRE( parse_device_index("cuda", index) );
	REQUIRE( index.get_backend_name() == "cuda" );
	REQUIRE( index.get_device_id() == 0 );
}

TEST_CASE( "parsing invalid device_index should return false", "[device_index]" )
{
	device_index index;
	REQUIRE( !parse_device_index("3:example", index) );
	REQUIRE( !parse_device_index(":3", index) );
	REQUIRE( !parse_device_index("test:", index) );
	REQUIRE( !parse_device_index(":", index) );
	REQUIRE( !parse_device_index("", index) );
}

TEST_CASE( "outputting device_index to ostream should produce the expected string", "[device_index]" )
{
	std::ostringstream oss;
	const device_index index("example", 4);
	oss << index;
	REQUIRE( oss.str() == "example:4" );
}
