// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/operation_descriptor.hpp>

#include <rexlib/core/operation/operand_names.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include <sstream>
#include <string>

using namespace rexlib;

namespace
{

const operation_descriptor& test_descriptor()
{
	static REXLIB_CONST_CONSTEXPR auto outputs =
		make_operand_names("quotient", "remainder");
	static REXLIB_CONST_CONSTEXPR auto inputs =
		make_operand_names("dividend", "divisor", "mode");
	static const operation_descriptor instance =
		make_operation_descriptor("rexlib.test", "divide", outputs, inputs);
	return instance;
}

} // anonymous namespace

TEST_CASE(
	"operation_descriptor should store its component and name",
	"[operation_descriptor]"
)
{
	const auto &descriptor = test_descriptor();

	CHECK( std::string(descriptor.get_component()) == "rexlib.test" );
	CHECK( std::string(descriptor.get_name()) == "divide" );
}

TEST_CASE(
	"operation_descriptor should name its operands",
	"[operation_descriptor]"
)
{
	const auto &descriptor = test_descriptor();

	REQUIRE( descriptor.get_output_operand_names().size() == 2 );
	REQUIRE( descriptor.get_input_operand_names().size() == 3 );
	CHECK( std::string(descriptor.get_operand_name(1, true)) ==
	       "remainder" );
	CHECK( std::string(descriptor.get_operand_name(2, false)) == "mode" );
	CHECK( descriptor.get_operand_name(2, true) == nullptr );
	CHECK( descriptor.get_operand_name(3, false) == nullptr );
}

TEST_CASE(
	"operation_descriptor should print as its qualified name",
	"[operation_descriptor]"
)
{
	std::ostringstream oss;
	oss << test_descriptor();

	CHECK( oss.str() == "rexlib.test.divide" );
}

TEST_CASE(
	"describe_operand should quote a name and fall back to the index",
	"[operation_descriptor]"
)
{
	const auto &descriptor = test_descriptor();

	CHECK( describe_operand(descriptor, 0, false) == "'dividend'" );
	CHECK( describe_operand(descriptor, 5, true) == "5" );
}
