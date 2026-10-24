// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/type_pivot_descriptor_table.hpp>

#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>
#include <rexlib/core/operation/type_rule/type_pivot.hpp>

using namespace rexlib;

namespace
{

struct first_accessor
{
};

struct second_accessor
{
};

} // anonymous namespace

TEST_CASE(
	"type_pivot_descriptor_table of no pivot should be empty",
	"[type_pivot_descriptor_table]"
)
{
	CHECK( type_pivot_descriptor_table<type_list<>>::get().empty() );
}

TEST_CASE(
	"type_pivot_descriptor_table should lower every pivot in order",
	"[type_pivot_descriptor_table]"
)
{
	using table = type_pivot_descriptor_table<
		type_list<
			pivot_from_input<1, floating_point_type_domain>,
			pivot_from_operation<first_accessor>,
			pivot_from_output<0, any_type_domain, 0>,
			pivot_from_operation<second_accessor, complex_type_domain>
		>
	>;

	const auto pivots = table::get();
	REQUIRE( pivots.size() == 4 );

	CHECK( pivots[0].get_source() == type_pivot_source::input );
	CHECK( pivots[0].get_index() == 1 );
	CHECK( pivots[0].get_default_pivot() == -1 );
	CHECK( &pivots[0].get_domain() == &floating_point_type_domain::get() );

	CHECK( pivots[1].get_source() == type_pivot_source::operation );
	CHECK( pivots[1].get_index() == 0 );

	CHECK( pivots[2].get_source() == type_pivot_source::output );
	CHECK( pivots[2].get_index() == 0 );
	CHECK( pivots[2].get_default_pivot() == 0 );

	CHECK( pivots[3].get_source() == type_pivot_source::operation );
	CHECK( pivots[3].get_index() == 1 );
	CHECK( &pivots[3].get_domain() == &complex_type_domain::get() );

	CHECK( table::get().data() == pivots.data() );
}
