// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/memory/align.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace rexlib;

TEST_CASE( "get_alignment returns correct alignment for address and pointer", "[align]" ) 
{
	std::uintptr_t address;
	std::size_t expected_alignment;
	std::tie(address, expected_alignment) = GENERATE(
		table<std::uintptr_t, std::size_t>({
			{0xABCD0001, 0x01},
			{0xABCD0002, 0x02},
			{0xABCD0100, 0x100},
			{0xABCD0200, 0x200},
			{0xABCD0400, 0x400},
			{0xABCD0800, 0x800}
		})
	);

	void* pointer = reinterpret_cast<void*>(address);

	REQUIRE( get_alignment(address) == expected_alignment );
	REQUIRE( get_alignment(pointer) == expected_alignment );
}

TEST_CASE( "is_aligned returns true only if the pointer is aligned to the requested boundary", "[align]" ) 
{
	std::uintptr_t address;
	std::size_t alignment;
	bool expected;
	std::tie(address, alignment, expected) = GENERATE(
		table<std::uintptr_t, std::size_t, bool>({
			{0xA1010100, 0x10,   true},
			{0xA1010100, 0x100,  true},
			{0xA1010100, 0x1000, false},
			{0xA1010111, 0x10,   false},
			{0xA1010111, 0x100,  false},
			{0xA1010111, 0x1000, false},
			{0x000000FF, 0x10,   false},
			{0x00001001, 0x100,  false}
		})
	);

	void* pointer = reinterpret_cast<void*>(address);

	REQUIRE( is_aligned(address, alignment) == expected );
	REQUIRE( is_aligned(pointer, alignment) == expected );
}

TEST_CASE( "align_floor aligns address and pointer down to nearest boundary", "[align]" ) 
{
	std::uintptr_t address;
	std::size_t alignment;
	std::uintptr_t expected;
	std::tie(address, alignment, expected) = GENERATE(
		table<std::uintptr_t, std::size_t, std::uintptr_t>({
			{0xA1010100, 0x10,   0xA1010100},
			{0xA1010100, 0x100,  0xA1010100},
			{0xA1010100, 0x1000, 0xA1010000},
			{0xA1010111, 0x10,   0xA1010110},
			{0xA1010111, 0x100,  0xA1010100},
			{0xA1010111, 0x1000, 0xA1010000},
			{0x000000FF, 0x10,   0x000000F0},
			{0x00001001, 0x100,  0x00001000}
		})
	);

	void* pointer = reinterpret_cast<void*>(address);

	REQUIRE( align_floor(address, alignment) == expected );
	REQUIRE( align_floor(pointer, alignment) == reinterpret_cast<void*>(expected) );
}

TEST_CASE( "align_ceil aligns address and pointer up to nearest boundary", "[align]" ) 
{
	std::uintptr_t address;
	std::size_t alignment;
	std::uintptr_t expected;
	std::tie(address, alignment, expected) = GENERATE(
		table<std::uintptr_t, std::size_t, std::uintptr_t>({
			{0xA1010100, 0x10,   0xA1010100},
			{0xA1010100, 0x100,  0xA1010100},
			{0xA1010100, 0x1000, 0xA1011000},
			{0xA1010111, 0x10,   0xA1010120},
			{0xA1010111, 0x100,  0xA1010200},
			{0xA1010111, 0x1000, 0xA1011000},
			{0x000000FF, 0x10,   0x00000100},
			{0x00001001, 0x100,  0x00001100}
		})
	);

	void* pointer = reinterpret_cast<void*>(address);

	REQUIRE( align_ceil(address, alignment) == expected );
	REQUIRE( align_ceil(pointer, alignment) == reinterpret_cast<void*>(expected) );
}

TEST_CASE( "offset_bytes computes pointer offset correctly for positive and negative values", "[align]" ) 
{
	std::uintptr_t base;
	std::ptrdiff_t offset;
	std::tie(base, offset) = GENERATE(
		table<std::uintptr_t, std::ptrdiff_t>({
			{0x4000, 0},
			{0x1000, 1},
			{0x2000, 2},
			{0x3000, 3},
			{0x1000, -1},
			{0x2000, -2},
			{0x3000, -3}
		})
	);

	std::uint32_t* data = reinterpret_cast<std::uint32_t*>(base);
	std::uint32_t* result = offset_bytes(data, offset*sizeof(std::uint32_t));
	REQUIRE( result == data + offset );
}
