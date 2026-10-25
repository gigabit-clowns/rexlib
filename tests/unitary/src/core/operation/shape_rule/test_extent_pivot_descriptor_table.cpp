// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/shape_rule/extent_pivot_descriptor_table.hpp>

#include <rexlib/core/operation/shape_rule/extent_pivot.hpp>
#include <rexlib/core/operation/shape_rule/positive_extent_domain.hpp>

using namespace rexlib;

namespace
{

struct test_count
{
};

} // anonymous namespace

TEST_CASE(
	"extent_pivot_descriptor_table of no variable should be empty",
	"[extent_pivot_descriptor_table]"
)
{
	CHECK( extent_pivot_descriptor_table<type_list<>>::get().empty() );
}

TEST_CASE(
	"extent_pivot_descriptor_table should lower every variable in order",
	"[extent_pivot_descriptor_table]"
)
{
	using table = extent_pivot_descriptor_table<
		type_list<
			extent_from_inputs<>,
			extent_from_operation<test_count, positive_extent_domain>
		>
	>;

	const auto extents = table::get();
	REQUIRE( extents.size() == 2 );
	CHECK( extents[0].get_source() == extent_pivot_source::inputs );
	CHECK( extents[0].get_domain().contains(0) );
	CHECK( extents[1].get_source() == extent_pivot_source::operation );
	CHECK_FALSE( extents[1].get_domain().contains(0) );

	CHECK( table::get().data() == extents.data() );
}
