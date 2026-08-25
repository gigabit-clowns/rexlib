// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/memory/byte_order.hpp>

using namespace rexlib;

TEST_CASE( "reverse_byte_order correctly reverses byte order for integral types", "[byte_order]" ) 
{
	SECTION( "int8_t" )
	{
		REQUIRE( reverse_byte_order(int8_t(0x12)) == 0x12 );
	}
	SECTION( "uint8_t" )
	{
		REQUIRE( reverse_byte_order(uint8_t(0x12)) == 0x12 );
	}
	SECTION( "int16_t" )
	{
		REQUIRE( reverse_byte_order(int16_t(0x1234)) == 0x3412 );
	}
	SECTION( "uint16_t" )
	{
		REQUIRE( reverse_byte_order(uint16_t(0x1234)) == 0x3412 );
	}
	SECTION( "int32_t" )
	{
		REQUIRE( reverse_byte_order(int32_t(0x12345678)) == 0x78563412 );
	}
	SECTION( "uint32_t" )
	{
		REQUIRE( reverse_byte_order(uint32_t(0x12345678)) == 0x78563412 );
	}
	SECTION( "int64_t" )
	{
		REQUIRE( reverse_byte_order(int64_t(0x123456789ABCDEF0)) == 0xF0DEBC9A78563412 );
	}
	SECTION( "uint64_t" )
	{
		REQUIRE( reverse_byte_order(uint64_t(0x123456789ABCDEF0)) == 0xF0DEBC9A78563412 );
	}
}
