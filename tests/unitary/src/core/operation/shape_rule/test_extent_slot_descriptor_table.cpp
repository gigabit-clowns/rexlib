// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/shape_rule/extent_slot_descriptor_table.hpp>

#include <rexlib/core/operation/shape_rule/extent_slot.hpp>

#include "fixtures/doubled_extent_transform.hpp"

#include <string>

using namespace rexlib;

TEST_CASE(
	"extent_slot_descriptor_table of no slot should be empty",
	"[extent_slot_descriptor_table]"
)
{
	CHECK( extent_slot_descriptor_table<type_list<>>::get().empty() );
}

TEST_CASE(
	"extent_slot_descriptor_table should lower every slot in order",
	"[extent_slot_descriptor_table]"
)
{
	using table = extent_slot_descriptor_table<
		type_list<test::axis_doubled_of<1>, axis_same_as<0>>
	>;

	const auto slots = table::get();
	REQUIRE( slots.size() == 2 );

	CHECK( slots[0].get_pivot_index() == 1 );
	CHECK( slots[0].get_transform().apply(8) == 16 );
	CHECK_FALSE( slots[0].get_transform().is_identity() );
	CHECK(
		std::string(slots[0].get_transform().get_relation()) ==
		test::doubled_extent_transform::get_relation()
	);

	CHECK( slots[1].get_pivot_index() == 0 );
	CHECK( slots[1].get_transform().apply(8) == 8 );
	CHECK( slots[1].get_transform().is_identity() );

	CHECK( table::get().data() == slots.data() );
}
