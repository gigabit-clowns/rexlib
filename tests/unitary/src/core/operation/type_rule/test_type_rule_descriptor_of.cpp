// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/type_rule_descriptor_of.hpp>

#include <rexlib/core/operation/type_rule/operand_type_rule.hpp>
#include <rexlib/core/operation/type_rule/type_pivot_descriptor_table.hpp>
#include <rexlib/core/operation/type_rule/type_slot_descriptor_table.hpp>

using namespace rexlib;

namespace
{

using test_rule = operand_type_rule<
	type_list<pivot_from_input<0>, pivot_from_output<0>>,
	type_list<slot_same_as<1>>,
	type_list<slot_same_as<0>, slot_real_of<0>>
>;

} // anonymous namespace

TEST_CASE(
	"type_rule_descriptor_of should lower a rule into its tables",
	"[type_rule_descriptor_of]"
)
{
	const auto &rule = type_rule_descriptor_of<test_rule>::get();

	CHECK(
		rule.get_pivots().data() ==
		type_pivot_descriptor_table<test_rule::pivot_list_type>::get().data()
	);
	CHECK( rule.get_pivots().size() == 2 );
	CHECK(
		rule.get_output_slots().data() ==
		type_slot_descriptor_table<
			test_rule::output_slot_list_type
		>::get().data()
	);
	CHECK( rule.get_output_slots().size() == 1 );
	CHECK(
		rule.get_input_slots().data() ==
		type_slot_descriptor_table<
			test_rule::input_slot_list_type
		>::get().data()
	);
	CHECK( rule.get_input_slots().size() == 2 );
}

TEST_CASE(
	"type_rule_descriptor_of should hand out one descriptor per rule",
	"[type_rule_descriptor_of]"
)
{
	CHECK(
		&type_rule_descriptor_of<test_rule>::get() ==
		&type_rule_descriptor_of<test_rule>::get()
	);
}
