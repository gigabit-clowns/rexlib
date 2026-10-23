// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/rules/shared_rule_contract.hpp>

#include "shape_rule_calls.hpp"

#include <rexlib/ops/rules/operand_shape_rules.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

#include <vector>

using namespace rexlib;

using test::shape_rule_test_descriptor;

namespace
{

using binary_contract = shared_rule_contract<
	ops::binary_elementwise_shape_rule,
	ops::binary_homogeneous_rule<>
>;

using unary_contract = shared_rule_contract<
	ops::unary_elementwise_shape_rule,
	ops::unary_homogeneous_rule<>
>;

} // anonymous namespace

TEST_CASE(
	"shared_rule_contract should hand out one contract per pair of rules",
	"[shared_rule_contract]"
)
{
	CHECK( &binary_contract::get() == &binary_contract::get() );
	CHECK(
		static_cast<const void*>(&binary_contract::get()) !=
		static_cast<const void*>(&unary_contract::get())
	);
}

TEST_CASE(
	"shared_rule_contract should deduce the outputs its rules call for",
	"[shared_rule_contract]"
)
{
	const std::vector<operand_specification> inputs = {
		operand_specification({ 4, 1 }, numerical_type::int16),
		operand_specification({ 3 }, numerical_type::int16)
	};
	std::vector<operand_specification> outputs(1);

	binary_contract::get().deduce(
		shape_rule_test_descriptor(),
		make_span(outputs),
		make_span(inputs)
	);
	CHECK(
		outputs[0] ==
		operand_specification({ 4, 3 }, numerical_type::int16)
	);
}
