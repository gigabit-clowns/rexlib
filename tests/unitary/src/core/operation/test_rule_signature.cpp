// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/rule_signature.hpp>

#include "shape_rule/fixtures/shape_rule_calls.hpp"

#include <rexlib/core/operation/type_rule/operand_type_rule.hpp>
#include <rexlib/ops/parameters/reduction_axes.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

#include <stdexcept>
#include <vector>

using namespace rexlib;

using test::shape_rule_test_descriptor;
using test::shape_type;

namespace
{

using reduction_signature = rule_signature<
	ops::reduction_shape_rule,
	ops::unary_homogeneous_rule<>,
	ops::reduction_axes
>;

class target_type_parameters
{
public:
	explicit target_type_parameters(numerical_type target) noexcept
		: m_target(target)
	{
	}

	numerical_type get_target() const noexcept
	{
		return m_target;
	}

private:
	numerical_type m_target;
};

struct target_of
{
	template <typename Parameters>
	static numerical_type get(const Parameters &parameters) noexcept
	{
		return parameters.get_target();
	}
};

using astype_rule = operand_type_rule<
	type_list<pivot_from_operation<target_of>, pivot_from_input<0>>,
	type_list<slot_same_as<0>>,
	type_list<slot_same_as<1>>
>;

using astype_signature = rule_signature<
	ops::unary_elementwise_shape_rule,
	astype_rule,
	target_type_parameters
>;

std::vector<operand_specification> one_float32(shape_type shape)
{
	return { operand_specification(std::move(shape), numerical_type::float32) };
}

} // anonymous namespace

TEST_CASE(
	"rule_signature should own the parameters it is built from",
	"[rule_signature]"
)
{
	const reduction_signature signature(ops::axis_list{ 1, 0 }, true);
	const auto axes = signature.get_parameters().get_axes();
	CHECK( ops::axis_list(axes.begin(), axes.end()) == ops::axis_list{ 1, 0 } );
	CHECK( signature.get_parameters().get_keep_dimensions() );
}

TEST_CASE(
	"rule_signature should deduce both the shape and the data type "
	"of every output",
	"[rule_signature]"
)
{
	const reduction_signature signature(ops::axis_list{ 0 }, true);
	const auto inputs = one_float32({ 2, 3 });
	std::vector<operand_specification> outputs(1);

	signature.deduce(
		shape_rule_test_descriptor(),
		make_span(outputs),
		make_span(inputs)
	);
	CHECK(
		outputs[0] ==
		operand_specification({ 1, 3 }, numerical_type::float32)
	);
}

TEST_CASE(
	"rule_signature should reject an output either rule rejects",
	"[rule_signature]"
)
{
	const reduction_signature signature(ops::axis_list{ 0 }, true);
	const auto inputs = one_float32({ 2, 3 });
	const auto canonical = one_float32({ 1, 3 });

	CHECK_NOTHROW(
		signature.accept(
			shape_rule_test_descriptor(),
			make_span(canonical),
			make_span(canonical),
			make_span(inputs)
		)
	);

	const auto wrong_shape = one_float32({ 3 });
	CHECK_THROWS_AS(
		signature.accept(
			shape_rule_test_descriptor(),
			make_span(wrong_shape),
			make_span(canonical),
			make_span(inputs)
		),
		std::invalid_argument
	);

	const std::vector<operand_specification> wrong_type = {
		operand_specification({ 1, 3 }, numerical_type::int32)
	};
	CHECK_THROWS_AS(
		signature.accept(
			shape_rule_test_descriptor(),
			make_span(wrong_type),
			make_span(canonical),
			make_span(inputs)
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"rule_signature should check its parameters against the rules",
	"[rule_signature]"
)
{
	const reduction_signature admitted(ops::axis_list{ 0, 1 });
	CHECK_NOTHROW( admitted.check_parameters(shape_rule_test_descriptor()) );

	const reduction_signature repeated(ops::axis_list{ 1, 1 });
	CHECK_THROWS_AS(
		repeated.check_parameters(shape_rule_test_descriptor()),
		std::invalid_argument
	);
}

TEST_CASE(
	"rule_signature should read the data types the operation "
	"supplies from its parameters",
	"[rule_signature]"
)
{
	const astype_signature signature(numerical_type::float64);
	CHECK_NOTHROW( signature.check_parameters(shape_rule_test_descriptor()) );

	const std::vector<operand_specification> inputs = {
		operand_specification({ 4 }, numerical_type::int32)
	};
	std::vector<operand_specification> outputs(1);
	signature.deduce(
		shape_rule_test_descriptor(),
		make_span(outputs),
		make_span(inputs)
	);
	CHECK(
		outputs[0] ==
		operand_specification({ 4 }, numerical_type::float64)
	);
}
