// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/type_rule_status.hpp>

#include <string>

using namespace rexlib;

TEST_CASE(
	"to_string should name every type_rule_status",
	"[type_rule_status]"
)
{
	CHECK( std::string(to_string(type_rule_status::ok)) == "ok" );
	CHECK( std::string(to_string(type_rule_status::invalid_type)) ==
	       "invalid type" );
	CHECK( std::string(to_string(type_rule_status::domain_violation)) ==
	       "domain violation" );
	CHECK( std::string(to_string(type_rule_status::undefined_transform)) ==
	       "undefined transform" );
	CHECK( std::string(to_string(type_rule_status::slot_mismatch)) ==
	       "slot mismatch" );
	CHECK( std::string(to_string(type_rule_status::unresolved_pivot)) ==
	       "unresolved pivot" );
	CHECK(
		std::string(to_string(type_rule_status::operand_count_mismatch)) ==
		"operand count mismatch"
	);
}
