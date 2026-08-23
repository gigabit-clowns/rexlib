// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/system/access_flags.hpp>

using namespace rex;

TEST_CASE( "to_string with access_flag_bits should produce correct results", "[access_flags]" ) 
{
	REQUIRE( std::string(to_string(access_flag_bits::read)) == "read" ); 
	REQUIRE( std::string(to_string(access_flag_bits::write)) == "write" ); 
}
