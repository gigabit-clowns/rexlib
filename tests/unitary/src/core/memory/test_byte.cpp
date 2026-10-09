// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/memory/byte.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <sstream>
#include <type_traits>

using namespace rexlib;

TEST_CASE( "a byte converts from and to its value", "[byte]" )
{
	const auto value = GENERATE(
		std::uint8_t(0x00),
		std::uint8_t(0x7F),
		std::uint8_t(0x80),
		std::uint8_t(0xFF)
	);

	CHECK( as_uint8(as_byte(value)) == value );
}

TEST_CASE( "as_bytes views the bytes an object is stored in", "[byte]" )
{
	const std::uint32_t value = 0x01020304;

	std::array<std::uint8_t, sizeof(value)> expected;
	std::memcpy(expected.data(), &value, sizeof(value));

	const auto *bytes = as_bytes(&value);
	for (std::size_t i = 0; i < sizeof(value); ++i)
	{
		CHECK( as_uint8(bytes[i]) == expected[i] );
	}
}

TEST_CASE( "as_bytes keeps the constness of what it views", "[byte]" )
{
	std::uint32_t value = 0;
	const std::uint32_t const_value = 0;

	CHECK( std::is_same<decltype(as_bytes(&value)), byte*>::value );
	CHECK(
		std::is_same<decltype(as_bytes(&const_value)), const byte*>::value
	);
}

TEST_CASE(
	"the bytes of an object show what is written to it afterwards",
	"[byte]"
)
{
	std::uint64_t value = 0;
	const auto *bytes = as_bytes(&value);

	// A byte may alias any object, so the write below cannot be assumed
	// away from the reads that follow it.
	value = 0xFFFFFFFFFFFFFFFFULL;

	for (std::size_t i = 0; i < sizeof(value); ++i)
	{
		CHECK( as_uint8(bytes[i]) == 0xFF );
	}
}

TEST_CASE( "an object shows what is written through its bytes", "[byte]" )
{
	std::uint64_t value = 0;
	auto *bytes = as_bytes(&value);

	for (std::size_t i = 0; i < sizeof(value); ++i)
	{
		bytes[i] = as_byte(0xFF);
	}

	CHECK( value == 0xFFFFFFFFFFFFFFFFULL );
}

TEST_CASE( "a byte shifts as its value does", "[byte]" )
{
	CHECK( as_uint8(as_byte(0x0F) << 4) == 0xF0 );
	CHECK( as_uint8(as_byte(0xF0) >> 4) == 0x0F );

	// Bits shifted past either end are lost.
	CHECK( as_uint8(as_byte(0xFF) << 4) == 0xF0 );
	CHECK( as_uint8(as_byte(0xFF) >> 4) == 0x0F );

	auto shifted = as_byte(0x0F);
	shifted <<= 2;
	CHECK( as_uint8(shifted) == 0x3C );
	shifted >>= 4;
	CHECK( as_uint8(shifted) == 0x03 );
}

TEST_CASE( "bytes combine bit by bit", "[byte]" )
{
	const auto lhs = as_byte(0xCC);
	const auto rhs = as_byte(0xAA);

	CHECK( as_uint8(~lhs) == 0x33 );
	CHECK( as_uint8(lhs | rhs) == 0xEE );
	CHECK( as_uint8(lhs & rhs) == 0x88 );
	CHECK( as_uint8(lhs ^ rhs) == 0x66 );

	auto combined = lhs;
	combined |= rhs;
	CHECK( as_uint8(combined) == 0xEE );
	combined &= rhs;
	CHECK( as_uint8(combined) == 0xAA );
	combined ^= lhs;
	CHECK( as_uint8(combined) == 0x66 );
}

TEST_CASE( "get_byte_bits reports the bits of a char", "[byte]" )
{
	CHECK( get_byte_bits() == CHAR_BIT );
}

TEST_CASE( "to_hex writes a byte as two upper case digits", "[byte]" )
{
	char high = 0;
	char low = 0;

	to_hex(as_byte(0xAB), high, low);
	CHECK( high == 'A' );
	CHECK( low == 'B' );

	to_hex(as_byte(0x09), high, low);
	CHECK( high == '0' );
	CHECK( low == '9' );
}

TEST_CASE( "a byte prints as its two hexadecimal digits", "[byte]" )
{
	std::ostringstream stream;
	stream << as_byte(0x3C) << as_byte(0x0F);

	CHECK( stream.str() == "3C0F" );
}

TEST_CASE( "a byte hashes to its value", "[byte]" )
{
	CHECK( std::hash<byte>()(as_byte(0x2A)) == 0x2A );
}
