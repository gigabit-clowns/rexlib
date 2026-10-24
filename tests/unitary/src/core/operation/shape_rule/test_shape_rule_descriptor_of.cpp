// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/shape_rule/shape_rule_descriptor_of.hpp>

#include <rexlib/core/operation/shape_rule/named_core.hpp>
#include <rexlib/core/operation/shape_rule/operand_shape_rule.hpp>

using namespace rexlib;

namespace
{

struct test_axes
{
};

using vector_dot_rule = operand_shape_rule<
	type_list<extent_from_inputs<>>,
	type_list<core_dimensions<>>,
	type_list<
		core_dimensions<axis_same_as<0>>,
		core_dimensions<axis_same_as<0>>
	>,
	named_core<test_axes>
>;

} // anonymous namespace

TEST_CASE(
	"shape_rule_descriptor_of should lower every part of a rule",
	"[shape_rule_descriptor_of]"
)
{
	const auto &descriptor = shape_rule_descriptor_of<vector_dot_rule>::get();

	REQUIRE( descriptor.get_pivots().size() == 1 );
	CHECK(
		descriptor.get_pivots()[0].get_source() ==
		extent_pivot_source::inputs
	);

	REQUIRE( descriptor.get_output_cores().size() == 1 );
	CHECK( descriptor.get_output_cores()[0].get_slots().empty() );

	REQUIRE( descriptor.get_input_cores().size() == 2 );
	CHECK( descriptor.get_input_cores()[0].get_slots().size() == 1 );
	CHECK(
		descriptor.get_input_cores()[1].get_slots()[0].get_pivot_index() == 0
	);

	CHECK( descriptor.get_placement() == core_placement::named );
	CHECK( descriptor.get_output_batch() == output_batch::exact );
}

TEST_CASE(
	"shape_rule_descriptor_of should lower a rule once",
	"[shape_rule_descriptor_of]"
)
{
	CHECK(
		&shape_rule_descriptor_of<vector_dot_rule>::get() ==
		&shape_rule_descriptor_of<vector_dot_rule>::get()
	);
}
