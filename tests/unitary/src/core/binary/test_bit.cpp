// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/binary/bit.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <limits>
#include <type_traits>
#include <cstdint>

using namespace rex;

TEST_CASE( "bit shifts left by 1", "[bit]" ) 
{
	std::size_t input;
	std::size_t expected_result;

	std::tie(input, expected_result) = GENERATE(
		table<std::size_t, std::size_t>({
			{0, 1},
			{1, 2},
			{2, 4},
			{8, 256},
			{std::numeric_limits<std::size_t>::digits-1, std::numeric_limits<std::make_signed<std::size_t>::type>::min()}
		})
	);

	REQUIRE( bit(input) == expected_result );
}

TEST_CASE( "bit_range_mask fills with 1s in the provided range", "[bit]" ) 
{
	std::size_t start;
	std::size_t stop;

	SECTION( "8 bits" )
	{
		std::uint8_t expected_result;
		std::tie(start, stop, expected_result) = GENERATE(
			table<std::size_t, std::size_t, std::uint8_t>({
				{ 0, 0, 0x00 },
				{ 0, 1, 0x01 },
				{ 0, 2, 0x03 },
				{ 0, 8, 0xFF },
				{ 1, 8, 0xFE },
				{ 4, 8, 0xF0 }
			})
		);

		REQUIRE( bit_range_mask<std::uint8_t>(start, stop) == expected_result );
	}

	SECTION( "16 bits" )
	{
		std::uint16_t expected_result;
		std::tie(start, stop, expected_result) = GENERATE(
			table<std::size_t, std::size_t, std::uint16_t>({
				{ 0,  0, 0x0000 },
				{ 0,  1, 0x0001 },
				{ 0,  2, 0x0003 },
				{ 0, 16, 0xFFFF },
				{ 1, 16, 0xFFFE },
				{ 8, 16, 0xFF00 },
				{ 4, 12, 0x0FF0 }
			})
		);

		REQUIRE( bit_range_mask<std::uint16_t>(start, stop) == expected_result );
	}

	SECTION( "32 bits" )
	{
		std::uint32_t expected_result;
		std::tie(start, stop, expected_result) = GENERATE(
			table<std::size_t, std::size_t, std::uint32_t>({
				{ 0,  0, 0x00000000 },
				{ 0,  1, 0x00000001 },
				{ 0,  2, 0x00000003 },
				{ 0, 32, 0xFFFFFFFF },
				{ 1, 32, 0xFFFFFFFE },
				{ 8, 32, 0xFFFFFF00 },
				{ 8, 24, 0x00FFFF00 }
			})
		);

		REQUIRE( bit_range_mask<std::uint32_t>(start, stop) == expected_result );
	}
}

TEST_CASE( "set_lowest_zero_inplace sets the lowest zero bit", "[bit]" ) 
{
	SECTION( "8 bits" )
	{
		uint8_t input, expected;
		SECTION( "set bits from LSB progressively" )
		{
			std::tie(input, expected) = GENERATE(
				table<uint8_t, uint8_t>({
					{0x00, 0x01},
					{0x01, 0x03},
					{0x03, 0x07},
					{0x07, 0x0F},
					{0x0F, 0x1F},
					{0x1F, 0x3F},
					{0x3F, 0x7F},
					{0x7F, 0xFF},
					{0xFF, 0xFF}
				})
			);
			set_lowest_zero_inplace(input);
			REQUIRE(input == expected);
		}
		SECTION( "set bits from LSB progressively when there is a bit set elsewhere" )
		{
			std::tie(input, expected) = GENERATE(
				table<uint8_t, uint8_t>({
					{0x0A, 0x0B},
					{0x0B, 0x0F},
					{0x0F, 0x1F}
				})
			);
			set_lowest_zero_inplace(input);
			REQUIRE(input == expected);
		}
		SECTION( "edge case, when there are no zeros to be set" )
		{
			input = std::numeric_limits<uint8_t>::max();
			set_lowest_zero_inplace(input);
			REQUIRE(input == std::numeric_limits<uint8_t>::max());
		}
	}

	SECTION( "32 bits" )
	{
		uint32_t input, expected;
		SECTION( "set bits from LSB progressively" )
		{
			std::tie(input, expected) = GENERATE(
				table<uint32_t, uint32_t>({
					{0x00, 0x01},
					{0x01, 0x03},
					{0x03, 0x07},
					{0x07, 0x0F},
					{0x0F, 0x1F},
					{0x1F, 0x3F},
					{0x3F, 0x7F},
					{0x7F, 0xFF},
					{0xFF, 0x1FF}
				})
			);
			set_lowest_zero_inplace(input);
			REQUIRE(input == expected);
		}
		SECTION( "set bits from LSB progressively when there is a bit set elsewhere" )
		{
			std::tie(input, expected) = GENERATE(
				table<uint32_t, uint32_t>({
					{0x0A, 0x0B},
					{0x0B, 0x0F},
					{0x0F, 0x1F}
				})
			);
			set_lowest_zero_inplace(input);
			REQUIRE(input == expected);
		}
		SECTION( "edge case, when there are no zeros to be set" )
		{
			input = std::numeric_limits<uint32_t>::max();
			set_lowest_zero_inplace(input);
			REQUIRE(input == std::numeric_limits<uint32_t>::max());
		}
	}

	SECTION( "64 bits" )
	{
		uint64_t input, expected;
		SECTION( "set bits from LSB progressively" )
		{
			std::tie(input, expected) = GENERATE(
				table<uint64_t, uint64_t>({
					{0x00, 0x01},
					{0x01, 0x03},
					{0x03, 0x07},
					{0x07, 0x0F},
					{0x0F, 0x1F},
					{0x1F, 0x3F},
					{0x3F, 0x7F},
					{0x7F, 0xFF},
					{0xFF, 0x1FF}
				})
			);
			set_lowest_zero_inplace(input);
			REQUIRE(input == expected);
		}
		SECTION( "set bits from LSB progressively when there is a bit set elsewhere" )
		{
			std::tie(input, expected) = GENERATE(
				table<uint64_t, uint64_t>({
					{0x0A, 0x0B},
					{0x0B, 0x0F},
					{0x0F, 0x1F}
				})
			);
			set_lowest_zero_inplace(input);
			REQUIRE(input == expected);
		}
		SECTION( "edge case, when there are no zeros to be set" )
		{
			input = std::numeric_limits<uint64_t>::max();
			set_lowest_zero_inplace(input);
			REQUIRE(input == std::numeric_limits<uint64_t>::max());
		}
	}
}

TEST_CASE( "clear_lowest_one_inplace clears the lowest one bit", "[bit]" ) 
{
	SECTION( "8 bits" )
	{
		uint8_t input, expected;
		SECTION( "clear bits from LSB progressively" )
		{
			std::tie(input, expected) = GENERATE(
				table<uint8_t, uint8_t>({
					{0xFF, 0xFE},
					{0xFE, 0xFC},
					{0xFC, 0xF8},
					{0xF8, 0xF0},
					{0xF0, 0xE0},
					{0xE0, 0xC0},
					{0xC0, 0x80},
					{0x80, 0x00}
				})
			);
			clear_lowest_one_inplace(input);
			REQUIRE(input == expected);
		}
		SECTION( "clear bits from LSB progressively when there is a bit clear elsewhere" )
		{
			std::tie(input, expected) = GENERATE(
				table<uint8_t, uint8_t>({
					{0xDB, 0xDA},
					{0xDA, 0xD8},
					{0xD8, 0xD0},
					{0xD0, 0xC0},
					{0xC0, 0x80}
				})
			);
			clear_lowest_one_inplace(input);
			REQUIRE(input == expected);
		}
		SECTION( "edge case, when there are no ones to be cleared" )
		{
			input = 0x00;
			clear_lowest_one_inplace(input);
			REQUIRE(input == 0x00);
		}
	}

	SECTION( "32 bits" )
	{
		uint32_t input, expected;
		SECTION( "clear bits from LSB progressively" )
		{
			std::tie(input, expected) = GENERATE(
				table<uint32_t, uint32_t>({
					{0xFF, 0xFE},
					{0xFE, 0xFC},
					{0xFC, 0xF8},
					{0xF8, 0xF0},
					{0xF0, 0xE0},
					{0xE0, 0xC0},
					{0xC0, 0x80},
					{0x80, 0x00}
				})
			);
			clear_lowest_one_inplace(input);
			REQUIRE(input == expected);
		}
		SECTION( "clear bits from LSB progressively when there is a bit clear elsewhere" )
		{
			std::tie(input, expected) = GENERATE(
				table<uint32_t, uint32_t>({
					{0xDB, 0xDA},
					{0xDA, 0xD8},
					{0xD8, 0xD0},
					{0xD0, 0xC0},
					{0xC0, 0x80}
				})
			);
			clear_lowest_one_inplace(input);
			REQUIRE(input == expected);
		}
		SECTION( "edge case, when there are no ones to be cleared" )
		{
			input = 0x00;
			clear_lowest_one_inplace(input);
			REQUIRE(input == 0x00);
		}
	}

	SECTION( "64 bits" )
	{
		uint64_t input, expected;
		SECTION( "clear bits from LSB progressively" )
		{
			std::tie(input, expected) = GENERATE(
				table<uint64_t, uint64_t>({
					{0xFF, 0xFE},
					{0xFE, 0xFC},
					{0xFC, 0xF8},
					{0xF8, 0xF0},
					{0xF0, 0xE0},
					{0xE0, 0xC0},
					{0xC0, 0x80},
					{0x80, 0x00}
				})
			);
			clear_lowest_one_inplace(input);
			REQUIRE(input == expected);
		}
		SECTION( "clear bits from LSB progressively when there is a bit clear elsewhere" )
		{
			std::tie(input, expected) = GENERATE(
				table<uint64_t, uint64_t>({
					{0xDB, 0xDA},
					{0xDA, 0xD8},
					{0xD8, 0xD0},
					{0xD0, 0xC0},
					{0xC0, 0x80}
				})
			);
			clear_lowest_one_inplace(input);
			REQUIRE(input == expected);
		}
		SECTION( "edge case, when there are no ones to be cleared" )
		{
			input = 0x00;
			clear_lowest_one_inplace(input);
			REQUIRE(input == 0x00);
		}
	}
}

TEST_CASE( "mask_trailing_zeros returns mask for trailing zeros", "[bit]" )
{
	SECTION( "8 bits" )
	{
		uint8_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint8_t, uint8_t>({
				{0x01, 0x00},
				{0x02, 0x01},
				{0x04, 0x03},
				{0xF4, 0x03},
				{0xF0, 0x0F},
				{0x00, 0xFF}
			})
		);
		REQUIRE(mask_trailing_zeros(input) == expected);
	}
	SECTION( "32 bits" )
	{
		uint32_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint32_t, uint32_t>({
				{0x00000001, 0x00000000},
				{0x00000002, 0x00000001},
				{0x00000004, 0x00000003},
				{0xFFF40000, 0x0003FFFF},
				{0xF0000000, 0x0FFFFFFF},
				{0x00000000, 0xFFFFFFFF}
			})
		);
		REQUIRE(mask_trailing_zeros(input) == expected);
	}
	SECTION( "64 bits" )
	{
		uint64_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint64_t, uint64_t>({
				{0x0000000000000001ULL, 0x0000000000000000ULL},
				{0x0000000000000002ULL, 0x0000000000000001ULL},
				{0x0000000000000004ULL, 0x0000000000000003ULL},
				{0xFFFFFFFFFFF40000ULL, 0x000000000003FFFFULL},
				{0xF000000000000000ULL, 0x0FFFFFFFFFFFFFFFULL},
				{0x0000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL}
			})
		);
		REQUIRE(mask_trailing_zeros(input) == expected);
	}
}

TEST_CASE( "mask_trailing_ones returns mask for trailing ones", "[bit]" )
{
	SECTION( "8 bits" )
	{
		uint8_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint8_t, uint8_t>({
				{0x00, 0x00},
				{0x01, 0x01},
				{0x03, 0x03},
				{0xF3, 0x03},
				{0xFF, 0xFF}
			})
		);
		REQUIRE(mask_trailing_ones(input) == expected);
	}
	SECTION( "32 bits" )
	{
		uint32_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint32_t, uint32_t>({
				{0x00000000, 0x00000000},
				{0x00000001, 0x00000001},
				{0x00000003, 0x00000003},
				{0xFFFFFFF3, 0x00000003},
				{0xFFFFFFFF, 0xFFFFFFFF}
			})
		);
		REQUIRE(mask_trailing_ones(input) == expected);
	}
	SECTION( "64 bits" )
	{
		uint64_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint64_t, uint64_t>({
				{0x0000000000000000ULL, 0x0000000000000000ULL},
				{0x0000000000000001ULL, 0x0000000000000001ULL},
				{0x0000000000000003ULL, 0x0000000000000003ULL},
				{0xFFFFFFFFFFFFFFF3ULL, 0x0000000000000003ULL},
				{0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL}
			})
		);
		REQUIRE(mask_trailing_ones(input) == expected);
	}
}

TEST_CASE( "rotate_right rotates bits to the right", "[bit]" )
{
	SECTION( "8 bits" )
	{
		uint8_t input, expected;
		int shift;
		std::tie(input, shift, expected) = GENERATE(
			table<uint8_t, int, uint8_t>({
				{0xDE, 0,  0xDE},
				{0xDE, 1,  0x6F},
				{0xDE, -1, 0xBD},
				{0xDE, 2,  0xB7},
				{0xDE, -2, 0x7B},
				{0xDE, 8,  0xDE},
				{0xDE, 9,  0x6F},
				{0xDE, -9, 0xBD},
				{0xDE, 10, 0xB7},
				{0xDE, -10,0x7B}
			})
		);
		REQUIRE(rotate_right(input, shift) == expected);
	}
	SECTION( "32 bits" )
	{
		uint32_t input, expected;
		int shift;
		std::tie(input, shift, expected) = GENERATE(
			table<uint32_t, int, uint32_t>({
				{0xDEADBEEF, 0,   0xDEADBEEF},
				{0xDEADBEEF, 1,   0xEF56DF77},
				{0xDEADBEEF, -1,  0xBD5B7DDF},
				{0xDEADBEEF, 2,   0xF7AB6FBB},
				{0xDEADBEEF, -2,  0x7AB6FBBF},
				{0xDEADBEEF, 32,  0xDEADBEEF},
				{0xDEADBEEF, 33,  0xEF56DF77},
				{0xDEADBEEF, -33, 0xBD5B7DDF},
				{0xDEADBEEF, 34,  0xF7AB6FBB},
				{0xDEADBEEF, -34, 0x7AB6FBBF}
			})
		);
		REQUIRE(rotate_right(input, shift) == expected);
	}
	SECTION( "64 bits" )
	{
		uint64_t input, expected;
		int shift;
		std::tie(input, shift, expected) = GENERATE(
			table<uint64_t, int, uint64_t>({
				{0xDEADBEEF01234567ULL, 0,    0xDEADBEEF01234567ULL},
				{0xDEADBEEF01234567ULL, 1,    0xEF56DF778091A2B3ULL},
				{0xDEADBEEF01234567ULL, -1,   0xBD5B7DDE02468ACFULL},
				{0xDEADBEEF01234567ULL, 2,    0xF7AB6FBBC048D159ULL},
				{0xDEADBEEF01234567ULL, -2,   0x7AB6FBBC048D159FULL},
				{0xDEADBEEF01234567ULL, 64,   0xDEADBEEF01234567ULL},
				{0xDEADBEEF01234567ULL, 65,   0xEF56DF778091A2B3ULL},
				{0xDEADBEEF01234567ULL, -65,  0xBD5B7DDE02468ACFULL},
				{0xDEADBEEF01234567ULL, 66,   0xF7AB6FBBC048D159ULL},
				{0xDEADBEEF01234567ULL, -66,  0x7AB6FBBC048D159FULL}
			})
		);
		REQUIRE(rotate_right(input, shift) == expected);
	}
}

TEST_CASE( "rotate_left rotates bits to the left", "[bit]" )
{
	SECTION( "8 bits" )
	{
		uint8_t input, expected;
		int shift;
		std::tie(input, shift, expected) = GENERATE(
			table<uint8_t, int, uint8_t>({
				{0xDE, 0,   0xDE},
				{0xDE, 1,   0xBD},
				{0xDE, -1,  0x6F},
				{0xDE, 2,   0x7B},
				{0xDE, -2,  0xB7},
				{0xDE, 8,   0xDE},
				{0xDE, 9,   0xBD},
				{0xDE, -9,  0x6F},
				{0xDE, 10,  0x7B},
				{0xDE, -10, 0xB7}
			})
		);
		REQUIRE(rotate_left(input, shift) == expected);
	}
	SECTION( "32 bits" )
	{
		uint32_t input, expected;
		int shift;
		std::tie(input, shift, expected) = GENERATE(
			table<uint32_t, int, uint32_t>({
				{0xDEADBEEF, 0,    0xDEADBEEF},
				{0xDEADBEEF, 1,    0xBD5B7DDF},
				{0xDEADBEEF, -1,   0xEF56DF77},
				{0xDEADBEEF, 2,    0x7AB6FBBF},
				{0xDEADBEEF, -2,   0xF7AB6FBB},
				{0xDEADBEEF, 32,   0xDEADBEEF},
				{0xDEADBEEF, 33,   0xBD5B7DDF},
				{0xDEADBEEF, -33,  0xEF56DF77},
				{0xDEADBEEF, 34,   0x7AB6FBBF},
				{0xDEADBEEF, -34,  0xF7AB6FBB}
			})
		);
		REQUIRE(rotate_left(input, shift) == expected);
	}
	SECTION( "64 bits" )
	{
		uint64_t input, expected;
		int shift;
		std::tie(input, shift, expected) = GENERATE(
			table<uint64_t, int, uint64_t>({
				{0xDEADBEEF01234567ULL, 0,    0xDEADBEEF01234567ULL},
				{0xDEADBEEF01234567ULL, -1,   0xEF56DF778091A2B3ULL},
				{0xDEADBEEF01234567ULL, 1,    0xBD5B7DDE02468ACFULL},
				{0xDEADBEEF01234567ULL, -2,   0xF7AB6FBBC048D159ULL},
				{0xDEADBEEF01234567ULL, 2,    0x7AB6FBBC048D159FULL},
				{0xDEADBEEF01234567ULL, 64,   0xDEADBEEF01234567ULL},
				{0xDEADBEEF01234567ULL, 65,   0xBD5B7DDE02468ACFULL},
				{0xDEADBEEF01234567ULL, -65,  0xEF56DF778091A2B3ULL},
				{0xDEADBEEF01234567ULL, 66,   0x7AB6FBBC048D159FULL},
				{0xDEADBEEF01234567ULL, -66,  0xF7AB6FBBC048D159ULL}
			})
		);
		REQUIRE(rotate_left(input, shift) == expected);
	}
}

TEST_CASE( "has_single_bit checks if value has a single bit set", "[bit]" )
{
	unsigned input;
	bool expected;
	std::tie(input, expected) = GENERATE(
		table<unsigned, bool>({
			{0U, false},
			{1U, true},
			{2U, true},
			{3U, false},
			{8U, true},
			{32U, true},
			{0x80000000U, true}
		})
	);
	REQUIRE(has_single_bit(input) == expected);
}

TEST_CASE( "bit_floor returns the largest power of two not greater than value", "[bit]" )
{
	unsigned input, expected;
	std::tie(input, expected) = GENERATE(
		table<unsigned, unsigned>({
			{0U, 0U},
			{1U, 1U},
			{2U, 2U},
			{3U, 2U},
			{4U, 4U},
			{5U, 4U},
			{31U, 16U},
			{0xFFFFFFFFU, 0x80000000U}
		})
	);
	REQUIRE(bit_floor(input) == expected);
}

TEST_CASE( "bit_ceil returns the smallest power of two not less than value", "[bit]" )
{
	unsigned input, expected;
	std::tie(input, expected) = GENERATE(
		table<unsigned, unsigned>({
			{0U, 1U},
			{1U, 1U},
			{2U, 2U},
			{3U, 4U},
			{4U, 4U},
			{5U, 8U},
			{31U, 32U}
		})
	);
	REQUIRE(bit_ceil(input) == expected);
}

TEST_CASE( "bit_width returns the minimum number of bits to represent value", "[bit]" )
{
	unsigned input, expected;
	std::tie(input, expected) = GENERATE(
		table<unsigned, unsigned>({
			{0U, 0U},
			{1U, 1U},
			{2U, 2U},
			{3U, 2U},
			{4U, 3U},
			{5U, 3U},
			{31U, 5U}
		})
	);
	REQUIRE(bit_width(input) == expected);
}

TEST_CASE( "count_leading_zeros counts the number of leading zeros", "[bit]" )
{
	SECTION( "8 bits" )
	{
		uint8_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint8_t, uint8_t>({
				{0x00, 8},
				{0x01, 7},
				{0x02, 6},
				{0x08, 4},
				{0x03, 6},
				{0x55, 1},
				{0xAA, 0},
				{0xFF, 0}
			})
		);
		REQUIRE(count_leading_zeros(input) == expected);
	}
	SECTION( "32 bits" )
	{
		uint32_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint32_t, uint32_t>({
				{0x00000000, 32},
				{0x00000001, 31},
				{0x00000002, 30},
				{0x00080000, 12},
				{0x03000000, 6},
				{0x55555555, 1},
				{0xAAAAAAAA, 0},
				{0xFFFFFFFF, 0}
			})
		);
		REQUIRE(count_leading_zeros(input) == expected);
	}
	SECTION( "64 bits" )
	{
		uint64_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint64_t, uint64_t>({
				{0x0000000000000000ULL, 64},
				{0x0000000000000001ULL, 63},
				{0x0000000000000002ULL, 62},
				{0x0008000000000000ULL, 12},
				{0x0300300000010000ULL, 6},
				{0x5555555555555555ULL, 1},
				{0xAAAAAAAAAAAAAAAAULL, 0},
				{0xFFFFFFFFFFFFFFFFULL, 0}
			})
		);
		REQUIRE(count_leading_zeros(input) == expected);
	}
}

TEST_CASE( "count_leading_ones counts the number of leading ones", "[bit]" )
{
	SECTION( "8 bits" )
	{
		uint8_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint8_t, uint8_t>({
				{0x00, 0},
				{0x88, 1},
				{0xC0, 2},
				{0xE2, 3},
				{0xF1, 4},
				{0x55, 0},
				{0xAA, 1},
				{0xF0, 4},
				{0xFF, 8}
			})
		);
		REQUIRE(count_leading_ones(input) == expected);
	}
	SECTION( "32 bits" )
	{
		uint32_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint32_t, uint32_t>({
				{0x00000000, 0},
				{0x88888888, 1},
				{0xCD063678, 2},
				{0xE2DFB245, 3},
				{0xF157D22D, 4},
				{0x55555555, 0},
				{0xAAAAAAAA, 1},
				{0xFFFFFFF0, 28},
				{0xFFFFFFFF, 32}
			})
		);
		REQUIRE(count_leading_ones(input) == expected);
	}
	SECTION( "64 bits" )
	{
		uint64_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint64_t, uint64_t>({
				{0x0000000000000000ULL, 0},
				{0x8888888888888888ULL, 1},
				{0xCD0612F3D43A6878ULL, 2},
				{0xE2DF124F6D4BB245ULL, 3},
				{0xF15782BF3D4DA22DULL, 4},
				{0x5555555555555555ULL, 0},
				{0xAAAAAAAAAAAAAAAAULL, 1},
				{0xFFFFFFFFFFFFFFF0ULL, 60},
				{0xFFFFFFFFFFFFFFFFULL, 64}
			})
		);
		REQUIRE(count_leading_ones(input) == expected);
	}
}

TEST_CASE( "count_trailing_zeros counts the number of trailing zeros", "[bit]" )
{
	SECTION( "8 bits" )
	{
		uint8_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint8_t, uint8_t>({
				{0x00, 8},
				{0x01, 0},
				{0x02, 1},
				{0x08, 3},
				{0x03, 0},
				{0x55, 0},
				{0xAA, 1},
				{0x10, 4},
				{0xFF, 0}
			})
		);
		REQUIRE(count_trailing_zeros(input) == expected);
	}
	SECTION( "32 bits" )
	{
		uint32_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint32_t, uint32_t>({
				{0x00000000, 32},
				{0x00000001, 0},
				{0x00000002, 1},
				{0x00080000, 19},
				{0x03000000, 24},
				{0x55555555, 0},
				{0xAAAAAAAA, 1},
				{0xFFFFFFFF, 0}
			})
		);
		REQUIRE(count_trailing_zeros(input) == expected);
	}
	SECTION( "64 bits" )
	{
		uint64_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint64_t, uint64_t>({
				{0x0000000000000000ULL, 64},
				{0x0000000000000001ULL, 0},
				{0x0000000000000002ULL, 1},
				{0x0008000000000000ULL, 51},
				{0x0300300000010000ULL, 16},
				{0x5555555555555555ULL, 0},
				{0xAAAAAAAAAAAAAAAAULL, 1},
				{0xFFFFFFFFFFFFFFFFULL, 0}
			})
		);
		REQUIRE(count_trailing_zeros(input) == expected);
	}
}

TEST_CASE( "count_trailing_ones counts the number of trailing ones", "[bit]" )
{
	SECTION( "8 bits" )
	{
		uint8_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint8_t, uint8_t>({
				{0x00, 0},
				{0x88, 0},
				{0x03, 2},
				{0x07, 3},
				{0x4F, 4},
				{0x55, 1},
				{0xAA, 0},
				{0x7F, 7},
				{0xFF, 8}
			})
		);
		REQUIRE(count_trailing_ones(input) == expected);
	}
	SECTION( "32 bits" )
	{
		uint32_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint32_t, uint32_t>({
				{0x00000000, 0},
				{0x88888888, 0},
				{0x02ABC001, 1},
				{0x02ABC003, 2},
				{0x55555555, 1},
				{0xAAAAAAAA, 0},
				{0x0FFFFFFF, 28},
				{0xFFFFFFFF, 32}
			})
		);
		REQUIRE(count_trailing_ones(input) == expected);
	}
	SECTION( "64 bits" )
	{
		uint64_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint64_t, uint64_t>({
				{0x0000000000000000ULL, 0},
				{0x8888888888888888ULL, 0},
				{0xCD0612F3D43A6873ULL, 2},
				{0xE2DF124F6D4BB247ULL, 3},
				{0xF15782BF3D4DA22FULL, 4},
				{0x5555555555555555ULL, 1},
				{0xAAAAAAAAAAAAAAAAULL, 0},
				{0x0FFFFFFFFFFFFFFFULL, 60},
				{0xFFFFFFFFFFFFFFFFULL, 64}
			})
		);
		REQUIRE(count_trailing_ones(input) == expected);
	}
}

TEST_CASE( "popcount counts the number of set bits", "[bit]" )
{
	SECTION( "8 bits" )
	{
		uint8_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint8_t, uint8_t>({
				{0x00, 0},
				{0x01, 1},
				{0x02, 1},
				{0x08, 1},
				{0x03, 2},
				{0x55, 4},
				{0xAA, 4},
				{0xFF, 8}
			})
		);
		REQUIRE(popcount(input) == expected);
	}
	SECTION( "32 bits" )
	{
		uint32_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint32_t, uint32_t>({
				{0x00000000, 0},
				{0x00000001, 1},
				{0x00000002, 1},
				{0x00080000, 1},
				{0x03000000, 2},
				{0x55555555, 16},
				{0xAAAAAAAA, 16},
				{0xFFFFFFFF, 32}
			})
		);
		REQUIRE(popcount(input) == expected);
	}
	SECTION( "64 bits" )
	{
		uint64_t input, expected;
		std::tie(input, expected) = GENERATE(
			table<uint64_t, uint64_t>({
				{0x0000000000000000ULL, 0},
				{0x0000000000000001ULL, 1},
				{0x0000000000000002ULL, 1},
				{0x0008000000000000ULL, 1},
				{0x0300300000010000ULL, 5},
				{0x5555555555555555ULL, 32},
				{0xAAAAAAAAAAAAAAAAULL, 32},
				{0xFFFFFFFFFFFFFFFFULL, 64}
			})
		);
		REQUIRE(popcount(input) == expected);
	}
}

TEST_CASE( "parity returns true if the number of set bits is odd", "[bit]" )
{
	SECTION( "8 bits" )
	{
		uint8_t input;
		bool expected;
		std::tie(input, expected) = GENERATE(
			table<uint8_t, bool>({
				{0x00, false},
				{0x01, true},
				{0x02, true},
				{0x08, true},
				{0x03, false},
				{0x55, false},
				{0xAA, false},
				{0xFF, false}
			})
		);
		REQUIRE(parity(input) == expected);
	}
	SECTION( "32 bits" )
	{
		uint32_t input;
		bool expected;
		std::tie(input, expected) = GENERATE(
			table<uint32_t, bool>({
				{0x00000000, false},
				{0x00000001, true},
				{0x00000002, true},
				{0x00080000, true},
				{0x03040000, true},
				{0x55555555, false},
				{0xAAAAAAAA, false},
				{0xFFFFFFFF, false}
			})
		);
		REQUIRE(parity(input) == expected);
	}
	SECTION( "64 bits" )
	{
		uint64_t input;
		bool expected;
		std::tie(input, expected) = GENERATE(
			table<uint64_t, bool>({
				{0x0000000000000000ULL, false},
				{0x0000000000000001ULL, true},
				{0x0000000000000002ULL, true},
				{0x0008000000000000ULL, true},
				{0x0300300000010000ULL, true},
				{0x5555555555555555ULL, false},
				{0xAAAAAAAAAAAAAAAAULL, false},
				{0xFFFFFFFFFFFFFFFFULL, false}
			})
		);
		REQUIRE(parity(input) == expected);
	}
}