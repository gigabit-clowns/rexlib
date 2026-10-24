// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/operand_names.hpp>

#include <rexlib/core/platform/constexpr.hpp>

#include <string>

using namespace rexlib;

TEST_CASE(
	"make_operand_names should keep the names in order",
	"[operand_names]"
)
{
	static REXLIB_CONST_CONSTEXPR auto names =
		make_operand_names("left", "right");

	const auto values = names.get();
	REQUIRE( values.size() == 2 );
	CHECK( std::string(values[0]) == "left" );
	CHECK( std::string(values[1]) == "right" );
}

TEST_CASE(
	"make_operand_names should build an empty list",
	"[operand_names]"
)
{
	static REXLIB_CONST_CONSTEXPR auto names = make_operand_names();

	CHECK( names.get().empty() );
}

TEST_CASE(
	"operand_names_count should count the names of a list",
	"[operand_names]"
)
{
	using names_type = decltype(make_operand_names("a", "b", "c"));

	STATIC_REQUIRE( operand_names_count<names_type>::value == 3 );
	STATIC_REQUIRE( operand_names_count<const names_type&>::value == 3 );
	STATIC_REQUIRE(
		operand_names_count<decltype(make_operand_names())>::value == 0
	);
}
