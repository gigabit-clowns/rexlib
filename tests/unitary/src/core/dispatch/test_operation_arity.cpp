// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/operation_arity.hpp>

using namespace rexlib;

TEST_CASE("Default constructed operation_arity should have no outputs and no inputs", "[operation_arity]")
{
	const operation_arity arity;
	CHECK( arity.get_output_count() == 0 );
	CHECK( arity.get_input_count() == 0 );
}

TEST_CASE("Constructing an operation_arity should store its operand counts", "[operation_arity]")
{
	const operation_arity arity(2, 3); // 2 outputs, 3 inputs
	CHECK( arity.get_output_count() == 2 );
	CHECK( arity.get_input_count() == 3 );
}

TEST_CASE("operation_arity-s with equal operand counts should compare as equal", "[operation_arity]")
{
	const operation_arity arity1(1, 2);
	const operation_arity arity2(1, 2);
	CHECK( arity1 == arity2 );
	CHECK( (arity1 != arity2) == false );
}

TEST_CASE("operation_arity-s should be unequal if either output or input count differ", "[operation_arity]")
{
	const operation_arity reference(1, 2);
	CHECK( reference != operation_arity(9, 2) ); // different output count
	CHECK( reference != operation_arity(1, 9) ); // different input count
	CHECK( reference != operation_arity(9, 9) ); // both differ
	CHECK( (reference == operation_arity(9, 2)) == false );
}

TEST_CASE("Copying an operation_arity should preserve its operand counts", "[operation_arity]")
{
	const operation_arity arity(4, 5);
	const operation_arity copy(arity);
	CHECK( copy == arity );
	CHECK( copy.get_output_count() == 4 );
	CHECK( copy.get_input_count() == 5 );
}

TEST_CASE("operation_arity presets should have one output and the expected input count", "[operation_arity]")
{
	CHECK( operation_arity::nullary() == operation_arity(1, 0) );
	CHECK( operation_arity::unary()   == operation_arity(1, 1) );
	CHECK( operation_arity::binary()  == operation_arity(1, 2) );
	CHECK( operation_arity::ternary() == operation_arity(1, 3) );
}
