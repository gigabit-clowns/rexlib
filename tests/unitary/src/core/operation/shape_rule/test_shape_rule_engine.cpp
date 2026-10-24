// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <core/operation/shape_rule/shape_rule_engine.hpp>

#include <rexlib/core/operation/shape_rule/named_core.hpp>
#include <rexlib/core/operation/shape_rule/operand_shape_rule.hpp>
#include <rexlib/core/operation/shape_rule/shape_rule_descriptor_of.hpp>

#include "fixtures/shape_rule_calls.hpp"

#include <stdexcept>
#include <vector>

using namespace rexlib;
using Catch::Matchers::ContainsSubstring;

namespace
{

using test::extent_list_type;
using test::make_shaped_operands;
using test::shape_rule_test_descriptor;
using test::shape_type;

struct test_axes
{
};

using cross_rule = operand_shape_rule<
	type_list<extent_fixed<3>>,
	type_list<core_dimensions<axis_same_as<0>>>,
	type_list<
		core_dimensions<axis_same_as<0>>,
		core_dimensions<axis_same_as<0>>
	>,
	named_core<test_axes>
>;

using reduction_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>>,
	type_list<core_dimensions<axis_run<>>>,
	named_core<test_axes>
>;

const shape_rule_descriptor& cross() noexcept
{
	return shape_rule_descriptor_of<cross_rule>::get();
}

} // anonymous namespace

TEST_CASE(
	"check_shape_rule_instance should require every extent the rule reads",
	"[shape_rule_engine]"
)
{
	const extent_list_type axes = { 1 };
	const extent_list_type none;
	const extent_list_type fixed = { 3 };

	CHECK_NOTHROW(
		check_shape_rule_instance(
			shape_rule_test_descriptor(),
			cross(),
			make_span(axes),
			make_span(fixed)
		)
	);
	CHECK_THROWS_WITH(
		check_shape_rule_instance(
			shape_rule_test_descriptor(),
			cross(),
			make_span(axes),
			make_span(none)
		),
		ContainsSubstring("the operation supplies 0 extents, but its shape "
			"rule reads 1.")
	);
}

TEST_CASE(
	"resolve_shape_rule should resolve the batch and the extents",
	"[shape_rule_engine]"
)
{
	const extent_list_type axes = { 1 };
	const extent_list_type fixed = { 3 };
	const auto inputs = make_shaped_operands({ { 4, 3 }, { 1, 3 } });

	const auto resolution = resolve_shape_rule(
		shape_rule_test_descriptor(),
		cross(),
		make_span(axes),
		make_span(fixed),
		1,
		make_span(inputs)
	);

	CHECK( resolution.get_batch() == shape_type{ 4 } );
	CHECK( resolution.get_extent(0) == 3 );
	CHECK( resolution.get_run_extents().empty() );
	CHECK(
		make_output_shape(cross(), resolution, make_span(axes), false, 0) ==
		shape_type{ 4, 3 }
	);
}

TEST_CASE(
	"resolve_shape_rule should take the extents of a run from the first "
	"input carrying it",
	"[shape_rule_engine]"
)
{
	const auto &rule = shape_rule_descriptor_of<reduction_rule>::get();
	const extent_list_type axes = { 0, 2 };
	const auto inputs = make_shaped_operands({ { 5, 6, 7 } });

	const auto resolution = resolve_shape_rule(
		shape_rule_test_descriptor(),
		rule,
		make_span(axes),
		span<const std::size_t>(),
		1,
		make_span(inputs)
	);

	CHECK( resolution.get_batch() == shape_type{ 6 } );
	CHECK( resolution.get_run_extents() == shape_type{ 5, 7 } );
	CHECK(
		make_output_shape(rule, resolution, make_span(axes), true, 0) ==
		shape_type{ 1, 6, 1 }
	);
}

TEST_CASE(
	"check_output_shapes should reject an output the inputs do not call for",
	"[shape_rule_engine]"
)
{
	const extent_list_type axes = { 1 };
	const extent_list_type fixed = { 3 };
	const auto inputs = make_shaped_operands({ { 4, 3 }, { 4, 3 } });
	const auto resolution = resolve_shape_rule(
		shape_rule_test_descriptor(),
		cross(),
		make_span(axes),
		make_span(fixed),
		1,
		make_span(inputs)
	);
	const auto good = make_shaped_operands({ { 4, 3 } });
	const auto bad = make_shaped_operands({ { 4, 2 } });

	CHECK_NOTHROW(
		check_output_shapes(
			shape_rule_test_descriptor(),
			cross(),
			resolution,
			make_span(axes),
			false,
			make_span(good)
		)
	);
	CHECK_THROWS_WITH(
		check_output_shapes(
			shape_rule_test_descriptor(),
			cross(),
			resolution,
			make_span(axes),
			false,
			make_span(bad)
		),
		ContainsSubstring("output operand 'result' has shape (4, 2), but the "
			"inputs call for (4, 3).")
	);
}
