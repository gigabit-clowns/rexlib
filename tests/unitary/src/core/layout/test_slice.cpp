// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/layout/slice.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <type_traits>
#include <sstream>

using namespace rexlib;

TEST_CASE( "make_slice with count argument should set count and leave the rest defaulted", "[slice]" ) 
{
	const auto count = GENERATE(std::size_t(0), std::size_t(1234), end());
	const auto s = make_slice(count);

	REQUIRE( s.get_start() == 0 );
	REQUIRE( s.get_count() == count );
	REQUIRE( s.get_step() == 1 );
}

TEST_CASE( "make_slice with start and count arguments should set their values and leave the rest defaulted", "[slice]" ) 
{
	const auto count = GENERATE(std::size_t(0), std::size_t(1234), end());
	const auto start = GENERATE(
		std::ptrdiff_t(0), 
		std::ptrdiff_t(1234), 
		std::ptrdiff_t(-2)
	);
	const auto s = make_slice(start, count);

	REQUIRE( s.get_start() == start );
	REQUIRE( s.get_count() == count );
	REQUIRE( s.get_step() == 1 );
}

TEST_CASE( "make_slice with start stop and step arguments should set their values", "[slice]" ) 
{
	const auto start = GENERATE(
		std::ptrdiff_t(0), 
		std::ptrdiff_t(1234), 
		std::ptrdiff_t(-2)
	);
	const auto count = GENERATE(std::size_t(0), std::size_t(1234), end());
	const auto step = GENERATE(
		std::ptrdiff_t(1), 
		std::ptrdiff_t(-10), 
		std::ptrdiff_t(12324)
	);
	const auto s = make_slice(start, count, step);

	REQUIRE( s.get_start() == start );
	REQUIRE( s.get_count() == count );
	REQUIRE( s.get_step() == step );
}

TEST_CASE( "output slice to a std::ostream should produce the expected strings", "[slice]" )
{
	std::ptrdiff_t start;
	std::size_t count;
	std::ptrdiff_t step;
	std::string expected_str;
	std::tie(start, count, step, expected_str) = GENERATE(
		table<std::ptrdiff_t, std::size_t, std::ptrdiff_t, std::string>({
			{0, 17, 1, "slice(0, 17, 1)" },
			{4, 3, 1, "slice(4, 3, 1)" },
			{-7, 0, 1, "slice(-7, 0, 1)" },
			{60, 10, -40, "slice(60, 10, -40)" }
		})
	);

	const auto s = make_slice(start, count, step);

	std::ostringstream oss;
	oss << s;
	REQUIRE( oss.str() == expected_str );
}

TEST_CASE( "sanitize_slice should throw with a stride of zero" )
{
	const slice input(2, 5, 0);
	const std::size_t extent = GENERATE(0, 1, 10, 20);

	REQUIRE_THROWS_MATCHES( 
		sanitize_slice(input, extent),
		std::invalid_argument,
		Catch::Matchers::Message("Slice step cannot be zero.")
	);
}

TEST_CASE( "sanitize_slice should pass correct positive start values" )
{
	const auto extent = 16;
	const std::ptrdiff_t count = 1;
	const std::ptrdiff_t step = GENERATE(1, -1, 10, -10, 30);
	const std::ptrdiff_t start = GENERATE(0, 8, 15);

	slice input(start, count, step);
	const auto output = sanitize_slice(input, extent);
	REQUIRE(output.get_start() == static_cast<std::size_t>(start));
	REQUIRE(output.get_count() == count);
	REQUIRE(output.get_step() == step);
}

TEST_CASE( "sanitize_slice should tolerate past-the-end start when forwards strided and zero-sized" )
{
	const std::ptrdiff_t start = GENERATE(0, 8, 16, 32);
	const std::size_t count = GENERATE(std::size_t(0), static_cast<std::size_t>(end()));
	const std::ptrdiff_t step = GENERATE(1, 2, 10);
	const auto extent = start;

	slice input(start, count, step);
	const auto output = sanitize_slice(input, extent);
	REQUIRE(output.get_start() == static_cast<std::size_t>(start));
}

TEST_CASE( "sanitize_slice should replace correct negative start values" )
{
	const auto extent = 16;
	const std::ptrdiff_t count = 1;
	const std::ptrdiff_t step = GENERATE(1, -1, 10, -10);
	std::ptrdiff_t start;
	std::size_t expected;
	std::tie(start, expected) = GENERATE(
		table<std::ptrdiff_t, std::size_t>({
			{-1, 15},
			{-4, 12},
			{-16, 0}
		})
	);
	
	slice input(start, count, step);
	const auto output = sanitize_slice(input, extent);
	REQUIRE(output.get_start() == expected);
	REQUIRE(output.get_count() == count);
	REQUIRE(output.get_step() == step);
}

TEST_CASE( "sanitize_slice should throw with out of bounds start values" )
{
	const auto extent = 16;
	const std::ptrdiff_t count = 1;
	std::ptrdiff_t step;
	std::ptrdiff_t start;
	std::string expected_error_msg;
	std::tie(start, step, expected_error_msg) = GENERATE(
		table<std::ptrdiff_t, std::ptrdiff_t, std::string>({
			{17, 1, "Slice's start index 17 is out of bounds for extent 16." },
			{17, 20, "Slice's start index 17 is out of bounds for extent 16." },
			{20, 1, "Slice's start index 20 is out of bounds for extent 16." },
			{99, 1, "Slice's start index 99 is out of bounds for extent 16." },
			{-17, 1, "Slice's negative start index -17 is out of bounds for extent 16." },
			{-17, -32, "Slice's negative start index -17 is out of bounds for extent 16." },
			{16, -1, "Backwards slice's start index 16 is out of bounds for extent 16." },
			{17, -1, "Backwards slice's start index 17 is out of bounds for extent 16." },
			{99, -1, "Backwards slice's start index 99 is out of bounds for extent 16." }
		})
	);
	
	slice input(start, count, step);
	REQUIRE_THROWS_MATCHES( 
		sanitize_slice(input, extent),
		std::out_of_range,
		Catch::Matchers::Message(expected_error_msg)
	);
}

TEST_CASE( "sanitize_slice should complete end values" )
{
	const auto extent = 16;
	const std::ptrdiff_t count = end();
	std::ptrdiff_t start;
	std::ptrdiff_t step;
	std::size_t expected_count;
	std::tie(start, step, expected_count) = GENERATE(
		table<std::ptrdiff_t, std::ptrdiff_t, std::size_t>({
			{ 0, 1, 16 },
			{ 1, 1, 15 },
			{ 4, 1, 12 },
			{ 15, 1, 1 },
			{ 0, 4, 4 },
			{ 3, 4, 4 },
			{ 4, 4, 3 },
			{ 15, 4, 1 },
			{ 15, -1, 16 },
			{ 14, -1, 15 },
			{ 12, -1, 13 },
			{ 0, -1, 1 },
			{ 15, -4, 4 },
			{ 12, -4, 4 },
			{ 11, -4, 3 },
			{ 0, -4, 1 }
		})
	);

	const slice input(start, count, step);
	const auto output = sanitize_slice(input, extent);
	REQUIRE(output.get_start() == start);
	REQUIRE(output.get_count() == expected_count);
	REQUIRE(output.get_step() == step);
}


TEST_CASE( "sanitize_slice should throw with an out of bounds slice count value" )
{
	const auto extent = 16;
	std::ptrdiff_t start;
	std::size_t count;
	std::ptrdiff_t step;
	std::string expected_error_msg;
	std::tie(start, count, step, expected_error_msg) = GENERATE(
		table<std::ptrdiff_t, std::size_t, std::ptrdiff_t, std::string>({
			{0, 17, 1, "Slice count 17 start index 0 and step 1 overflows extent 16" },
			{2, 15, 1, "Slice count 15 start index 2 and step 1 overflows extent 16" },
			{0, 5, 4, "Slice count 5 start index 0 and step 4 overflows extent 16"},
			{6, 3, 5, "Slice count 3 start index 6 and step 5 overflows extent 16"},
			{15, 17, -1, "Reversed slice with count 17 start index 15 and step -1 underflows 0"},
			{10, 12, -1, "Reversed slice with count 12 start index 10 and step -1 underflows 0"},
			{4, 3, -4, "Reversed slice with count 3 start index 4 and step -4 underflows 0"}
		})
	);
	
	slice input(start, count, step);
	REQUIRE_THROWS_MATCHES( 
		sanitize_slice(input, extent),
		std::out_of_range,
		Catch::Matchers::Message(expected_error_msg)
	);
}
