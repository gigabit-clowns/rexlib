// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/layout/dynamic_subscript.hpp>

#include "mock/mock_subscript_callable.hpp"

#include <type_traits>
#include <sstream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

using namespace rex;

TEST_CASE("construct dynamic_subscript from ellipsis_tag", "[dynamic_subscript]")
{
	const auto s = dynamic_subscript(ellipsis());
	REQUIRE( s.get_subscript_type() == dynamic_subscript::subscript_type::ellipsis );
}

TEST_CASE("construct dynamic_subscript from new_axis_tag", "[dynamic_subscript]")
{
	const auto s = dynamic_subscript(new_axis());
	REQUIRE( s.get_subscript_type() == dynamic_subscript::subscript_type::new_axis );
}

TEST_CASE("construct dynamic_subscript from index", "[dynamic_subscript]")
{
	const auto index = 1234;
	const auto s = dynamic_subscript(index);
	REQUIRE( s.get_subscript_type() == dynamic_subscript::subscript_type::index );
	REQUIRE( s.get_index() == index );
}

TEST_CASE("construct dynamic_subscript from constant index", "[dynamic_subscript]")
{
	const auto index = std::integral_constant<int, 1234>();
	const auto s = dynamic_subscript(index);
	REQUIRE( s.get_subscript_type() == dynamic_subscript::subscript_type::index );
	REQUIRE( s.get_index() == index );
}

TEST_CASE("calling get_index on a dynamic_subscript that holds another value should throw", "[dynamic_subscript]")
{
	dynamic_subscript s(new_axis());
	REQUIRE_THROWS_MATCHES( 
		s.get_index(),
		bad_dynamic_subscript_access,
		Catch::Matchers::Message("Cannot call get_index on a dynamic_subscript(new_axis_tag)")
	);
}

TEST_CASE("construct dynamic_subscript from slice", "[dynamic_subscript]")
{
	const auto slice = make_slice(1, 2, 3);
	const auto s = dynamic_subscript(slice);
	REQUIRE( s.get_subscript_type() == dynamic_subscript::subscript_type::slice );
	REQUIRE( s.get_slice() == slice );
}

TEST_CASE("construct dynamic_subscript from special slice", "[dynamic_subscript]")
{
	const auto s = dynamic_subscript(all());
	REQUIRE( s.get_subscript_type() == dynamic_subscript::subscript_type::slice );
	REQUIRE( s.get_slice() == all() );
}

TEST_CASE("copy-construct dynamic_subscript from slice", "[dynamic_subscript]")
{
	const auto slice = make_slice(1, 2, 3);
	const auto s0 = dynamic_subscript(slice);
	const dynamic_subscript s1(s0);
	REQUIRE( s1.get_subscript_type() == dynamic_subscript::subscript_type::slice );
	REQUIRE( s1.get_slice() == slice );
}

TEST_CASE("copy-assign dynamic_subscript from slice", "[dynamic_subscript]")
{
	const auto slice = make_slice(1, 2, 3);
	const auto s0 = dynamic_subscript(slice);
	auto s1 = dynamic_subscript(new_axis());
	s1 = s0;
	REQUIRE( s1.get_subscript_type() == dynamic_subscript::subscript_type::slice );
	REQUIRE( s1.get_slice() == slice );
}

TEST_CASE("calling get_slice on a dynamic_subscript that holds another value should throw", "[dynamic_subscript]")
{
	dynamic_subscript s(ellipsis());
	REQUIRE_THROWS_MATCHES( 
		s.get_slice(),
		bad_dynamic_subscript_access,
		Catch::Matchers::Message("Cannot call get_slice on a dynamic_subscript(ellipsis_tag)")
	);
}

TEST_CASE("visit function with a dynamic_subscript that holds an ellipsis_tag", "[dynamic_subscript]")
{
	mock_subscript_callable mock;
	REQUIRE_CALL(mock, function_call(ellipsis()));

	const auto s = dynamic_subscript(ellipsis());
	visit(mock, s);
}

TEST_CASE("visit function with a dynamic_subscript that holds a new_axis_tag", "[dynamic_subscript]")
{
	mock_subscript_callable mock;
	REQUIRE_CALL(mock, function_call(new_axis()));

	const auto s = dynamic_subscript(new_axis());
	visit(mock, s);
}

TEST_CASE("visit function with a dynamic_subscript that holds an index", "[dynamic_subscript]")
{
	mock_subscript_callable mock;
	const std::ptrdiff_t index = 12345;
	REQUIRE_CALL(mock, function_call(index));

	const auto s = dynamic_subscript(index);
	visit(mock, s);
}

TEST_CASE("visit function with a dynamic_subscript that holds a slice", "[dynamic_subscript]")
{
	mock_subscript_callable mock;
	const auto slice = make_slice(1, 2, 3);
	REQUIRE_CALL(mock, function_call(slice));

	const auto s = dynamic_subscript(slice);
	visit(mock, s);
}

TEST_CASE("outputing a dynamic_subscript to an ostream", "[dynamic_subscript]")
{
	std::ostringstream oss;
	oss << dynamic_subscript(ellipsis()) << "\n";
	oss << dynamic_subscript(new_axis()) << "\n";
	oss << dynamic_subscript(make_slice(1, 2, 3)) << "\n";
	oss << dynamic_subscript(10);

	const auto result = oss.str();
	const auto expected = 
		"dynamic_subscript(ellipsis_tag)\n"
		"dynamic_subscript(new_axis_tag)\n"
		"dynamic_subscript(slice(1, 2, 3))\n"
		"dynamic_subscript(10)";

	REQUIRE( result == expected );
}
