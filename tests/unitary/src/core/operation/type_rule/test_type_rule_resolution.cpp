// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/type_rule_resolution.hpp>

using namespace rexlib;

TEST_CASE(
	"type_rule_resolution should start as a success without pivots",
	"[type_rule_resolution]"
)
{
	const type_rule_resolution resolution;

	CHECK( resolution );
	CHECK( resolution.get_status() == type_rule_status::ok );
	CHECK( resolution.get_pivot_count() == 0 );
	CHECK( resolution.get_offending_type() == numerical_type::unknown );
}

TEST_CASE(
	"type_rule_resolution should hold the pivots it is given",
	"[type_rule_resolution]"
)
{
	type_rule_resolution resolution;
	resolution.set_pivot_count(2);

	CHECK( resolution.get_pivot(0) == numerical_type::unknown );
	CHECK( resolution.get_pivot(1) == numerical_type::unknown );

	resolution.set_pivot(1, numerical_type::float32);

	CHECK( resolution.get_pivot_count() == 2 );
	CHECK( resolution.get_pivot(1) == numerical_type::float32 );
}

TEST_CASE(
	"type_rule_resolution::failure should describe the failure",
	"[type_rule_resolution]"
)
{
	const auto resolution = type_rule_resolution::failure(
		type_rule_status::domain_violation,
		2,
		true,
		numerical_type::int8
	);

	CHECK_FALSE( resolution );
	CHECK( resolution.get_status() == type_rule_status::domain_violation );
	CHECK( resolution.get_operand_index() == 2 );
	CHECK( resolution.is_output_operand() );
	CHECK( resolution.get_offending_type() == numerical_type::int8 );
	CHECK( resolution.get_pivot_count() == 0 );
}

TEST_CASE(
	"type_rule_resolution::fail should keep the pivots resolved so far",
	"[type_rule_resolution]"
)
{
	type_rule_resolution resolution;
	resolution.set_pivot_count(1);
	resolution.set_pivot(0, numerical_type::float64);

	const auto failed = resolution.fail(
		type_rule_status::slot_mismatch,
		1,
		false,
		numerical_type::int32
	);

	CHECK( resolution );
	CHECK_FALSE( failed );
	CHECK( failed.get_status() == type_rule_status::slot_mismatch );
	CHECK( failed.get_operand_index() == 1 );
	CHECK_FALSE( failed.is_output_operand() );
	CHECK( failed.get_offending_type() == numerical_type::int32 );
	REQUIRE( failed.get_pivot_count() == 1 );
	CHECK( failed.get_pivot(0) == numerical_type::float64 );
}
