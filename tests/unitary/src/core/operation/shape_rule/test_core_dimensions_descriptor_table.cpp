// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/shape_rule/core_dimensions_descriptor_table.hpp>

#include <rexlib/core/operation/shape_rule/core_dimensions.hpp>
#include <rexlib/core/operation/shape_rule/positive_extent_domain.hpp>

using namespace rexlib;

TEST_CASE(
	"core_dimensions_descriptor_table of no operand should be empty",
	"[core_dimensions_descriptor_table]"
)
{
	CHECK( core_dimensions_descriptor_table<type_list<>>::get().empty() );
}

TEST_CASE(
	"core_dimensions_descriptor_table should lower the core of every "
	"operand in order",
	"[core_dimensions_descriptor_table]"
)
{
	using table = core_dimensions_descriptor_table<
		type_list<
			core_dimensions<>,
			core_dimensions<axis_run<positive_extent_domain>, axis_same_as<0>>
		>
	>;

	const auto cores = table::get();
	REQUIRE( cores.size() == 2 );

	CHECK( cores[0].get_slots().empty() );
	CHECK_FALSE( cores[0].has_run() );

	REQUIRE( cores[1].get_slots().size() == 1 );
	CHECK( cores[1].get_slots()[0].get_pivot_index() == 0 );
	CHECK( cores[1].has_run() );
	CHECK_FALSE( cores[1].get_run_domain().contains(0) );

	CHECK( table::get().data() == cores.data() );
}
