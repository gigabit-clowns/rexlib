// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/shape_rule/operand_shape_rule.hpp>

#include <rexlib/core/operation/shape_rule/named_core.hpp>
#include <rexlib/core/operation/shape_rule/positive_extent_domain.hpp>

#include <type_traits>

using namespace rexlib;

namespace
{

struct test_axes
{
};

struct test_count
{
};

using matrix_product_rule = operand_shape_rule<
	type_list<extent_from_inputs<>, extent_from_inputs<>, extent_from_inputs<>>,
	type_list<core_dimensions<axis_same_as<0>, axis_same_as<2>>>,
	type_list<
		core_dimensions<axis_same_as<0>, axis_same_as<1>>,
		core_dimensions<axis_same_as<1>, axis_same_as<2>>
	>
>;

using reduction_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>>,
	type_list<core_dimensions<axis_run<>>>,
	named_core<test_axes>
>;

using sequence_rule = operand_shape_rule<
	type_list<extent_from_operation<test_count, positive_extent_domain>>,
	type_list<core_dimensions<axis_same_as<0>>>,
	type_list<>
>;

using elementwise_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>>,
	type_list<core_dimensions<>, core_dimensions<>>,
	trailing_core,
	output_batch::broadcast_target
>;

} // anonymous namespace

TEST_CASE(
	"operand_shape_rule should count its extent variables and operands",
	"[operand_shape_rule]"
)
{
	STATIC_CHECK( matrix_product_rule::extent_count == 3 );
	STATIC_CHECK( matrix_product_rule::output_count == 1 );
	STATIC_CHECK( matrix_product_rule::input_count == 2 );

	STATIC_CHECK( sequence_rule::extent_count == 1 );
	STATIC_CHECK( sequence_rule::input_count == 0 );
}

TEST_CASE(
	"operand_shape_rule should place the core axes last and require exact "
	"output batches by default",
	"[operand_shape_rule]"
)
{
	STATIC_CHECK(
		std::is_same<
			matrix_product_rule::placement_type,
			trailing_core
		>::value
	);
	STATIC_CHECK( matrix_product_rule::batch == output_batch::exact );
	STATIC_CHECK(
		elementwise_rule::batch == output_batch::broadcast_target
	);
}

TEST_CASE(
	"operand_shape_rule should require parameters only when it reads values "
	"of the operation instance",
	"[operand_shape_rule]"
)
{
	STATIC_CHECK_FALSE( matrix_product_rule::requires_parameters );
	STATIC_CHECK_FALSE( elementwise_rule::requires_parameters );
	STATIC_CHECK( reduction_rule::requires_parameters );
	STATIC_CHECK( sequence_rule::requires_parameters );
}
