// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/version.hpp>

using namespace rex;

TEST_CASE( "version constructor should produce expected values", "[version]" ) 
{
	version v(1234, 567, 890);
	REQUIRE( v.get_major() == 1234 );
	REQUIRE( v.get_minor() == 567 );
	REQUIRE( v.get_patch() == 890 );
}

TEST_CASE( "version copy constructor should produce expected values", "[version]" ) 
{
	version v(2345, 678, 901);
	version v2(v);
	REQUIRE( v2.get_major() == 2345 );
	REQUIRE( v2.get_minor() == 678 );
	REQUIRE( v2.get_patch() == 901 );
}

TEST_CASE( "version setters should modify stored values", "[version]" ) 
{
	version v(1, 2, 3);
	v.set_major(3971);
	v.set_minor(999);
	v.set_patch(997);
	REQUIRE( v.get_major() == 3971 );
	REQUIRE( v.get_minor() == 999 );
	REQUIRE( v.get_patch() == 997 );
}

TEST_CASE( "version should overflow with very large values", "[version]" ) 
{
	version v(4096, 1024, 1024);
	REQUIRE( v.get_major() == 0 );
	REQUIRE( v.get_minor() == 0 );
	REQUIRE( v.get_patch() == 0 );

	v.set_major(8193);
	v.set_minor(2049);
	v.set_patch(2049);
	REQUIRE( v.get_major() == 1 );
	REQUIRE( v.get_minor() == 1 );
	REQUIRE( v.get_patch() == 1 );
}
