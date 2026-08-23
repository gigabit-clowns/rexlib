// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/binary/flagset.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <limits>
#include <type_traits>
#include <cstdint>

#include <rex/core/binary/bit.hpp>

using namespace rex;

// Definition of the flag bits and its flagset

enum class test_int32_flag_bits : int32_t
{
	a = bit(0),
	b = bit(1),
	c = bit(2),

	// ...

	x = bit(28),
	y = bit(29),
	z = bit(30),
};
using test_int32_flags = flagset<test_int32_flag_bits>;


TEST_CASE( "flagset empty constructor should not have any flags", "[flagset]" ) 
{
	test_int32_flags flags;

	test_int32_flag_bits flag;
	bool expected_result;
	std::tie(flag, expected_result) = GENERATE(
		table<test_int32_flag_bits, bool>({
			{test_int32_flag_bits::a, false},
			{test_int32_flag_bits::b, false},
			{test_int32_flag_bits::c, false},
			{test_int32_flag_bits::x, false},
			{test_int32_flag_bits::y, false},
			{test_int32_flag_bits::z, false}
		})
	);
		
	REQUIRE(flags.contains(flag) == expected_result );
}

TEST_CASE( "flagset constructor from a bit should not have the provided bit", "[flagset]" ) 
{
	test_int32_flags flags = test_int32_flag_bits::x;

	test_int32_flag_bits flag;
	bool expected_result;
	std::tie(flag, expected_result) = GENERATE(
		table<test_int32_flag_bits, bool>({
			{test_int32_flag_bits::a, false},
			{test_int32_flag_bits::b, false},
			{test_int32_flag_bits::c, false},
			{test_int32_flag_bits::x, true},
			{test_int32_flag_bits::y, false},
			{test_int32_flag_bits::z, false}
		})
	);
		
	REQUIRE(flags.contains(flag) == expected_result );
}

TEST_CASE( "flagset constructor from raw should not have the provided bits", "[flagset]" ) 
{
	test_int32_flags flags(0x50000005);

	test_int32_flag_bits flag;
	bool expected_result;
	std::tie(flag, expected_result) = GENERATE(
		table<test_int32_flag_bits, bool>({
			{test_int32_flag_bits::a, true},
			{test_int32_flag_bits::b, false},
			{test_int32_flag_bits::c, true},
			{test_int32_flag_bits::x, true},
			{test_int32_flag_bits::y, false},
			{test_int32_flag_bits::z, true}
		})
	);

	REQUIRE(flags.contains(flag) == expected_result );
}

TEST_CASE( "flagset constructor from initializer should not have the provided bits", "[flagset]" ) 
{
	test_int32_flags flags = {
		test_int32_flag_bits::b,
		test_int32_flag_bits::y,
		test_int32_flag_bits::y, // Duplicate intentionally
		test_int32_flag_bits::z,
	};

	test_int32_flag_bits flag;
	bool expected_result;
	std::tie(flag, expected_result) = GENERATE(
		table<test_int32_flag_bits, bool>({
			{test_int32_flag_bits::a, false},
			{test_int32_flag_bits::b, true},
			{test_int32_flag_bits::c, false},
			{test_int32_flag_bits::x, false},
			{test_int32_flag_bits::y, true},
			{test_int32_flag_bits::z, true}
		})
	);

	REQUIRE(flags.contains(flag) == expected_result );
}

TEST_CASE("flagset bool cast", "[flagset]") 
{
	test_int32_flags flags = {
		test_int32_flag_bits::b,
		test_int32_flag_bits::y,
		test_int32_flag_bits::z,
	};
	REQUIRE(static_cast<bool>(flags) == true);

	flags = {};
	REQUIRE(static_cast<bool>(flags) == false);
}

TEST_CASE("flagset raw cast", "[flagset]") 
{
	test_int32_flags flags = {
		test_int32_flag_bits::b,
		test_int32_flag_bits::y,
		test_int32_flag_bits::z,
	};
	REQUIRE(static_cast<int32_t>(flags) == 0x60000002);
}

TEST_CASE("flagset contains", "[flagset]") 
{
	test_int32_flags flags = {
		test_int32_flag_bits::b,
		test_int32_flag_bits::y,
		test_int32_flag_bits::z,
	};
	REQUIRE(flags.contains(test_int32_flag_bits::a) == false);
	REQUIRE(flags.contains(test_int32_flag_bits::b) == true);
	REQUIRE(flags.contains(test_int32_flag_bits::c) == false);
	REQUIRE(flags.contains(test_int32_flag_bits::x) == false);
	REQUIRE(flags.contains(test_int32_flag_bits::y) == true);
	REQUIRE(flags.contains(test_int32_flag_bits::z) == true);
}

TEST_CASE("flagset all_of", "[flagset]") 
{
	test_int32_flags flags = {
		test_int32_flag_bits::b,
		test_int32_flag_bits::y,
		test_int32_flag_bits::z,
	};
	REQUIRE(flags.all_of({test_int32_flag_bits::b, test_int32_flag_bits::z}) == true);
	REQUIRE(flags.all_of({test_int32_flag_bits::b, test_int32_flag_bits::x}) == false);
}

TEST_CASE("flagset any_of", "[flagset]") 
{
	test_int32_flags flags = {
		test_int32_flag_bits::b,
		test_int32_flag_bits::y,
		test_int32_flag_bits::z,
	};
	REQUIRE(flags.any_of({test_int32_flag_bits::b, test_int32_flag_bits::x}) == true);
	REQUIRE(flags.any_of({test_int32_flag_bits::c, test_int32_flag_bits::x}) == false);
}

TEST_CASE("flagset none_of", "[flagset]") 
{
	test_int32_flags flags = {
		test_int32_flag_bits::b,
		test_int32_flag_bits::y,
		test_int32_flag_bits::z,
	};
	REQUIRE(flags.none_of({test_int32_flag_bits::a, test_int32_flag_bits::x}) == true);
	REQUIRE(flags.none_of({test_int32_flag_bits::a, test_int32_flag_bits::z}) == false);
}

TEST_CASE("flagset only_of", "[flagset]") 
{
	test_int32_flags flags = {
		test_int32_flag_bits::b,
		test_int32_flag_bits::y,
		test_int32_flag_bits::z,
	};
	REQUIRE(flags.only_of({test_int32_flag_bits::b, test_int32_flag_bits::y, test_int32_flag_bits::z}) == true);
	REQUIRE(flags.only_of({test_int32_flag_bits::b, test_int32_flag_bits::y, test_int32_flag_bits::z, test_int32_flag_bits::x }) == true);
	REQUIRE(flags.only_of({test_int32_flag_bits::a, test_int32_flag_bits::z}) == false);
}

TEST_CASE("flagset count and parity", "[flagset]") 
{
	test_int32_flags flags = {
		test_int32_flag_bits::b,
		test_int32_flag_bits::y,
		test_int32_flag_bits::z,
	};
	REQUIRE(flags.count() == 3);
	REQUIRE(flags.parity() == true);

	flags.set(test_int32_flag_bits::x);
	REQUIRE(flags.count() == 4);
	REQUIRE(flags.parity() == false);
	
	flags.clear();
	REQUIRE(flags.count() == 0);
	REQUIRE(flags.parity() == false);
}

TEST_CASE("flagset has single bit", "[flagset]") 
{
	test_int32_flags flags = {
		test_int32_flag_bits::b,
	};
	REQUIRE(flags.has_single_bit() == true);
	
	flags.set(test_int32_flag_bits::a);
	REQUIRE(flags.has_single_bit() == false);

	flags.clear();
	REQUIRE(flags.has_single_bit() == false);
}

TEST_CASE("flagset set and clear", "[flagset]") 
{
	test_int32_flags flags = {
		test_int32_flag_bits::b,
	};
	flags.set({test_int32_flag_bits::a, test_int32_flag_bits::b, test_int32_flag_bits::c}, true);
	REQUIRE(flags == test_int32_flags({test_int32_flag_bits::a, test_int32_flag_bits::b, test_int32_flag_bits::c}));
	
	flags.set({test_int32_flag_bits::a, test_int32_flag_bits::x}, false);
	REQUIRE(flags == test_int32_flags({test_int32_flag_bits::b, test_int32_flag_bits::c}));
}

TEST_CASE("flagset toggle", "[flagset]") 
{
	test_int32_flags flags = {
		test_int32_flag_bits::b,
	};
	flags.toggle({test_int32_flag_bits::a, test_int32_flag_bits::b, test_int32_flag_bits::c});
	REQUIRE(flags == test_int32_flags({test_int32_flag_bits::a, test_int32_flag_bits::c}));
}
