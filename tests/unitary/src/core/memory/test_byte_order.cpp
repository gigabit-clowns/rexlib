// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/memory/byte_order.hpp>

#include <rexlib/core/numerical/fixed_width_float.hpp>

#include <algorithm>
#include <complex>
#include <cstdint>
#include <cstring>
#include <vector>

using namespace rexlib;

namespace
{

// Reverses the bytes of a value without going through the function under
// test, so that the two are compared rather than one restating the other.
template <typename T>
T reversed(const T &value)
{
	unsigned char raw[sizeof(T)];
	std::memcpy(raw, &value, sizeof(raw));
	std::reverse(raw, raw + sizeof(raw));

	T result;
	std::memcpy(&result, raw, sizeof(result));
	return result;
}

template <typename T>
std::vector<unsigned char> bytes_of(const T &value)
{
	std::vector<unsigned char> raw(sizeof(T));
	std::memcpy(raw.data(), &value, raw.size());
	return raw;
}

} // anonymous namespace

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
		const auto expected = static_cast<int64_t>(0xF0DEBC9A78563412ULL);
		REQUIRE( reverse_byte_order(int64_t(0x123456789ABCDEF0)) == expected );
	}
	SECTION( "uint64_t" )
	{
		REQUIRE( reverse_byte_order(uint64_t(0x123456789ABCDEF0)) == 0xF0DEBC9A78563412 );
	}
}

TEST_CASE(
	"reverse_byte_order reverses a floating point value through its bits",
	"[byte_order]"
)
{
	SECTION( "single precision" )
	{
		const float value = 1.5F;

		REQUIRE( bytes_of(reverse_byte_order(value)) ==
			bytes_of(reversed(value)) );
	}

	SECTION( "double precision" )
	{
		const double value = -2.25;

		REQUIRE( bytes_of(reverse_byte_order(value)) ==
			bytes_of(reversed(value)) );
	}

	SECTION( "half precision" )
	{
		const float16_t value(1.5F);
		const auto swapped = reverse_byte_order(value);

		REQUIRE( swapped.get_bits() ==
			static_cast<std::uint16_t>(
				(value.get_bits() << 8) | (value.get_bits() >> 8)) );
	}

	SECTION( "reversing twice is the identity" )
	{
		const float single = -2.25F;
		const double twice = 1.0 / 3.0;

		REQUIRE( reverse_byte_order(reverse_byte_order(single)) == single );
		REQUIRE( reverse_byte_order(reverse_byte_order(twice)) == twice );
	}
}

TEST_CASE(
	"reverse_byte_order reverses a complex value one component at a time",
	"[byte_order]"
)
{
	const std::complex<float> value(1.5F, -2.25F);
	const auto swapped = reverse_byte_order(value);

	SECTION( "each component is reversed on its own" )
	{
		REQUIRE( bytes_of(swapped.real()) ==
			bytes_of(reversed(value.real())) );
		REQUIRE( bytes_of(swapped.imag()) ==
			bytes_of(reversed(value.imag())) );
	}

	SECTION( "it is not the reversal of the whole value" )
	{
		// A complex value is two numbers, not one twice as wide, so
		// reversing all of its bytes at once would also exchange its
		// components.
		REQUIRE( bytes_of(swapped) != bytes_of(reversed(value)) );
	}

	SECTION( "reversing twice is the identity" )
	{
		REQUIRE( reverse_byte_order(swapped) == value );
	}
}
