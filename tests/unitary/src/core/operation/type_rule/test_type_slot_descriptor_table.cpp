// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/type_slot_descriptor_table.hpp>

#include <rexlib/core/operation/type_rule/type_slot.hpp>

#include <string>

using namespace rexlib;

TEST_CASE(
	"type_slot_descriptor_table of no slot should be empty",
	"[type_slot_descriptor_table]"
)
{
	CHECK( type_slot_descriptor_table<type_list<>>::get().empty() );
}

TEST_CASE(
	"type_slot_descriptor_table should lower every slot in order",
	"[type_slot_descriptor_table]"
)
{
	using table = type_slot_descriptor_table<
		type_list<
			slot_same_as<0>,
			slot_real_of<1>,
			slot_fixed<numerical_type::boolean>
		>
	>;

	const auto slots = table::get();
	REQUIRE( slots.size() == 3 );

	CHECK( slots[0].get_pivot_index() == 0 );
	const auto &same = slots[0].get_transform();
	CHECK( same.apply(numerical_type::int8) == numerical_type::int8 );
	CHECK( std::string(same.get_relation()) == "the same type as" );

	CHECK( slots[1].get_pivot_index() == 1 );
	const auto &real = slots[1].get_transform();
	CHECK( real.apply(numerical_type::complex_float64) ==
	       numerical_type::float64 );
	CHECK( std::string(real.get_relation()) == "the real counterpart of" );

	CHECK( slots[2].get_pivot_index() == 0 );
	const auto &fixed = slots[2].get_transform();
	CHECK( fixed.apply(numerical_type::float32) == numerical_type::boolean );
	CHECK( std::string(fixed.get_relation()) == "independent of" );

	CHECK( table::get().data() == slots.data() );
}
