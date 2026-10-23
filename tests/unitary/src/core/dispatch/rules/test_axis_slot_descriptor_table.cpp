// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/rules/axis_slot_descriptor_table.hpp>

#include <rexlib/core/dispatch/rules/positive_extent_domain.hpp>

#include <string>

using namespace rexlib;

TEST_CASE(
	"axis_slot_descriptor_table of no slot should be empty",
	"[axis_slot_descriptor_table]"
)
{
	CHECK( axis_slot_descriptor_table<type_list<>>::get().empty() );
}

TEST_CASE(
	"axis_slot_descriptor_table should lower every slot in order",
	"[axis_slot_descriptor_table]"
)
{
	using table = axis_slot_descriptor_table<
		type_list<
			axis_run<positive_extent_domain>,
			axis_half_spectrum_of<1>,
			axis_fixed<3>,
			axis_same_as<0>
		>
	>;

	const auto slots = table::get();
	REQUIRE( slots.size() == 4 );

	CHECK( slots[0].get_kind() == axis_slot_kind::run );
	CHECK_FALSE( slots[0].get_domain().contains(0) );

	CHECK( slots[1].get_kind() == axis_slot_kind::extent );
	CHECK( slots[1].get_index() == 1 );
	CHECK( slots[1].apply(8) == 5 );
	CHECK_FALSE( slots[1].determines_extent() );
	CHECK(
		std::string(slots[1].get_relation()) ==
		half_spectrum_extent_transform::get_relation()
	);

	CHECK( slots[2].get_kind() == axis_slot_kind::fixed );
	CHECK( slots[2].get_index() == 3 );

	CHECK( slots[3].get_kind() == axis_slot_kind::extent );
	CHECK( slots[3].get_index() == 0 );
	CHECK( slots[3].determines_extent() );

	CHECK( table::get().data() == slots.data() );
}
