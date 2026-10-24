// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/operand_type_rule.hpp>

using namespace rexlib;

namespace
{

struct test_accessor
{
};

using input_rule = operand_type_rule<
	type_list<pivot_from_input<0>>,
	type_list<slot_same_as<0>>,
	type_list<slot_same_as<0>, slot_same_as<0>>
>;

using operation_rule = operand_type_rule<
	type_list<pivot_from_operation<test_accessor>, pivot_from_input<0>>,
	type_list<slot_same_as<0>>,
	type_list<slot_same_as<1>>
>;

} // anonymous namespace

TEST_CASE(
	"operand_type_rule should count its pivots and operands",
	"[operand_type_rule]"
)
{
	STATIC_CHECK( input_rule::pivot_count == 1 );
	STATIC_CHECK( input_rule::output_count == 1 );
	STATIC_CHECK( input_rule::input_count == 2 );
	STATIC_CHECK( operation_rule::pivot_count == 2 );
}

TEST_CASE(
	"operand_type_rule should require parameters only when it reads data "
	"types from the operation",
	"[operand_type_rule]"
)
{
	STATIC_CHECK_FALSE( input_rule::requires_parameters );
	STATIC_CHECK( operation_rule::requires_parameters );
}
