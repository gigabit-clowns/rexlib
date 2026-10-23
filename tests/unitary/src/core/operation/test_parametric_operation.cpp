// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/operation_declaration_macros.hpp>

#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/parameters/reduction_axes.hpp>
#include <rexlib/ops/rules/operand_shape_rules.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

#include <stdexcept>
#include <type_traits>
#include <vector>

using namespace rexlib;

namespace
{

REXLIB_DECLARE_PARAMETRIC_OPERATION(
	stub_reduce,
	ops::ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	ops::reduction_shape_rule,
	ops::reduction_axes,
	ops::unary_homogeneous_rule<>
);

operand_specification::shape_type deduce_shape(
	const operation &op,
	const operand_specification::shape_type &input_shape
)
{
	const std::vector<operand_specification> inputs = {
		operand_specification(input_shape, numerical_type::float32)
	};
	std::vector<operand_specification> outputs(1);
	op.get_contract().deduce(
		op.get_descriptor(),
		make_span(outputs),
		make_span(inputs)
	);
	return outputs[0].get_shape();
}

} // anonymous namespace

TEST_CASE(
	"a parametric operation should own its contract per instance",
	"[parametric_operation]"
)
{
	const stub_reduce_operation first(ops::axis_list{ 0 });
	const stub_reduce_operation last(ops::axis_list{ 2 });

	CHECK( &first.get_contract() != &last.get_contract() );
	CHECK(
		deduce_shape(first, { 2, 3, 4 }) ==
		operand_specification::shape_type{ 3, 4 }
	);
	CHECK(
		deduce_shape(last, { 2, 3, 4 }) ==
		operand_specification::shape_type{ 2, 3 }
	);
}

TEST_CASE(
	"a parametric operation should expose its parameters with their own type",
	"[parametric_operation]"
)
{
	const stub_reduce_operation op(ops::axis_list{ 1, 0 }, true);

	STATIC_REQUIRE( std::is_same<
		decltype(op.get_parameters()),
		const ops::reduction_axes&
	>::value );

	const auto axes = op.get_parameters().get_axes();
	CHECK( ops::axis_list(axes.begin(), axes.end()) == ops::axis_list{ 1, 0 } );
	CHECK( op.get_parameters().get_keep_dimensions() );
}

TEST_CASE(
	"a parametric operation should reject parameters its rules do not admit",
	"[parametric_operation]"
)
{
	CHECK_THROWS_AS(
		stub_reduce_operation(ops::axis_list{ 1, 1 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"a parametric operation should keep everything a declared one has",
	"[parametric_operation]"
)
{
	const stub_reduce_operation op(ops::axis_list{ 0 });

	CHECK( op.get_name() == "stub_reduce" );
	CHECK( op.get_arity() == operation_arity::unary() );
	CHECK( op.get_id() == operation_id::of<stub_reduce_operation>() );
	STATIC_REQUIRE( stub_reduce_operation::output_operand_count == 1 );
	STATIC_REQUIRE( stub_reduce_operation::input_operand_count == 1 );
	STATIC_REQUIRE( std::is_final<stub_reduce_operation>::value );
	STATIC_REQUIRE(
		std::is_base_of<operation, stub_reduce_operation>::value
	);

	const stub_reduce_operation other(ops::axis_list{ 1 });
	CHECK( &op.get_descriptor() == &other.get_descriptor() );
}
