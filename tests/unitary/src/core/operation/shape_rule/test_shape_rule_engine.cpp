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

using keyed_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<axis_run<>>>,
	type_list<core_dimensions<axis_run<>>, core_dimensions<>>,
	named_core<test_axes>
>;

using leading_key_rule = operand_shape_rule<
	type_list<extent_fixed<3>>,
	type_list<core_dimensions<axis_same_as<0>>>,
	type_list<core_dimensions<>, core_dimensions<axis_same_as<0>>>,
	named_core<test_axes>
>;

const shape_rule_descriptor& keyed() noexcept
{
	return shape_rule_descriptor_of<keyed_rule>::get();
}

shape_type resolve_keyed_batch(
	const extent_list_type &axes,
	const shape_type &value,
	const shape_type &key
)
{
	const auto inputs = make_shaped_operands({ value, key });
	return resolve_shape_rule(
		shape_rule_test_descriptor(),
		keyed(),
		make_span(axes),
		span<const std::size_t>(),
		1,
		make_span(inputs)
	).get_batch();
}

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

TEST_CASE(
	"resolve_shape_rule should broadcast an input without core axes against "
	"the batch of the inputs carrying the named axes",
	"[shape_rule_engine]"
)
{
	const extent_list_type trailing = { 2, 3 };

	CHECK(
		resolve_keyed_batch(trailing, { 2, 3, 6, 7 }, {}) ==
		shape_type{ 2, 3 }
	);
	CHECK(
		resolve_keyed_batch(trailing, { 2, 3, 6, 7 }, { 3 }) ==
		shape_type{ 2, 3 }
	);
	CHECK(
		resolve_keyed_batch(trailing, { 2, 3, 6, 7 }, { 2, 1 }) ==
		shape_type{ 2, 3 }
	);
	CHECK(
		resolve_keyed_batch(trailing, { 1, 3, 6, 7 }, { 2, 3 }) ==
		shape_type{ 2, 3 }
	);
}

TEST_CASE(
	"resolve_shape_rule should align an input without core axes to the axes "
	"that are not named, wherever the named axes lie",
	"[shape_rule_engine]"
)
{
	const extent_list_type axes = { 0, 2 };
	const auto inputs = make_shaped_operands({ { 6, 5, 7 }, { 5 } });

	const auto resolution = resolve_shape_rule(
		shape_rule_test_descriptor(),
		keyed(),
		make_span(axes),
		span<const std::size_t>(),
		1,
		make_span(inputs)
	);

	CHECK( resolution.get_batch() == shape_type{ 5 } );
	CHECK( resolution.get_run_extents() == shape_type{ 6, 7 } );
	CHECK(
		make_output_shape(keyed(), resolution, make_span(axes), false, 0) ==
		shape_type{ 6, 5, 7 }
	);
}

TEST_CASE(
	"resolve_shape_rule should report the axes of an input without core "
	"axes that does not broadcast",
	"[shape_rule_engine]"
)
{
	const extent_list_type axes = { 0, 2 };

	CHECK_THROWS_WITH(
		resolve_keyed_batch(axes, { 6, 5, 7 }, { 4 }),
		ContainsSubstring("input operands 'left' and 'right' do not "
			"broadcast together: axis 1 of the first has extent 5 and axis "
			"0 of the second has extent 4.")
	);
}

TEST_CASE(
	"resolve_shape_rule should reject an input without core axes that has "
	"more axes than the batch of the inputs carrying the named axes",
	"[shape_rule_engine]"
)
{
	const extent_list_type axes = { 1, 2 };

	CHECK_THROWS_WITH(
		resolve_keyed_batch(axes, { 5, 6, 7 }, { 2, 5 }),
		ContainsSubstring("input operand 'right' has 2 axes, but the inputs "
			"carrying the named axes have only 1 other axis for it to "
			"broadcast against.")
	);
	CHECK_THROWS_WITH(
		resolve_keyed_batch({ 0, 1 }, { 6, 7 }, { 5 }),
		ContainsSubstring("input operand 'right' has 1 axis, but the inputs "
			"carrying the named axes have only 0 other axes for it to "
			"broadcast against.")
	);
}

TEST_CASE(
	"check_shape_rule_instance should count the named axes against the first "
	"input carrying them",
	"[shape_rule_engine]"
)
{
	const auto &rule = shape_rule_descriptor_of<leading_key_rule>::get();
	const extent_list_type one = { 1 };
	const extent_list_type two = { 0, 1 };
	const extent_list_type fixed = { 3 };

	CHECK_NOTHROW(
		check_shape_rule_instance(
			shape_rule_test_descriptor(),
			rule,
			make_span(one),
			make_span(fixed)
		)
	);
	CHECK_THROWS_WITH(
		check_shape_rule_instance(
			shape_rule_test_descriptor(),
			rule,
			make_span(two),
			make_span(fixed)
		),
		ContainsSubstring("2 axes are named, but this operation acts along "
			"exactly 1 axis.")
	);
}
