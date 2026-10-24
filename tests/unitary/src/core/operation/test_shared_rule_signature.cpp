// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/shared_rule_signature.hpp>

#include "shape_rule/fixtures/shape_rule_calls.hpp"

#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

#include <vector>

using namespace rexlib;

using test::shape_rule_test_descriptor;

namespace
{

using binary_signature = shared_rule_signature<
	ops::binary_elementwise_shape_rule,
	ops::binary_homogeneous_rule<>
>;

using unary_signature = shared_rule_signature<
	ops::unary_elementwise_shape_rule,
	ops::unary_homogeneous_rule<>
>;

} // anonymous namespace

TEST_CASE(
	"shared_rule_signature should hand out one signature per pair of rules",
	"[shared_rule_signature]"
)
{
	CHECK( &binary_signature::get() == &binary_signature::get() );
	CHECK(
		static_cast<const void*>(&binary_signature::get()) !=
		static_cast<const void*>(&unary_signature::get())
	);
}

TEST_CASE(
	"shared_rule_signature should deduce the outputs its rules call for",
	"[shared_rule_signature]"
)
{
	const std::vector<operand_specification> inputs = {
		operand_specification({ 4, 1 }, numerical_type::int16),
		operand_specification({ 3 }, numerical_type::int16)
	};
	std::vector<operand_specification> outputs(1);

	binary_signature::get().deduce(
		shape_rule_test_descriptor(),
		make_span(outputs),
		make_span(inputs)
	);
	CHECK(
		outputs[0] ==
		operand_specification({ 4, 3 }, numerical_type::int16)
	);
}
