// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/operation_declaration_macros.hpp>

#include <rexlib/core/numerical/scalar_value.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

#include <array>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

using namespace rexlib;

namespace
{

// The usual case: the whole declaration of an operation.
REXLIB_DECLARE_OPERATION(
	stub_add,
	ops::ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("left", "right"),
	ops::binary_elementwise_shape_rule,
	ops::binary_homogeneous_rule<>
);

// Another operation following the same rules.
REXLIB_DECLARE_OPERATION(
	stub_subtract,
	ops::ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("left", "right"),
	ops::binary_elementwise_shape_rule,
	ops::binary_homogeneous_rule<>
);

// The escape hatch: an operation carrying a parameter still declares its
// description with the macro, and only writes by hand what is genuinely its
// own.
REXLIB_DECLARE_OPERATION_TRAITS(
	stub_fill,
	ops::ops_component,
	REXLIB_OPERANDS("destination"),
	REXLIB_OPERANDS(),
	ops::nullary_elementwise_shape_rule,
	ops::nullary_free_rule<>
);

class stub_fill_operation final
	: public trivial_operation<stub_fill_operation, stub_fill_operation_traits>
{
public:
	explicit stub_fill_operation(const scalar_value &fill_value) noexcept
		: m_fill_value(fill_value)
	{
	}

	const scalar_value& get_fill_value() const noexcept
	{
		return m_fill_value;
	}

private:
	scalar_value m_fill_value;
};

} // anonymous namespace

TEST_CASE(
    "a declared operation should carry the signature of its rules",
    "[trivial_operation]"
)
{
    const stub_add_operation op;

    const std::vector<operand_specification> inputs = {
        operand_specification({ 4 }, numerical_type::float32),
        operand_specification({ 1 }, numerical_type::float32)
    };
    std::vector<operand_specification> outputs(1);

    op.get_signature().deduce(
        op.get_descriptor(),
        make_span(outputs),
        make_span(inputs)
    );
    CHECK(
        outputs[0] == operand_specification({ 4 }, numerical_type::float32)
    );
}

TEST_CASE(
    "operations following the same rules should share one signature",
    "[trivial_operation]"
)
{
    const stub_add_operation add;
    const stub_subtract_operation subtract;

    CHECK( &add.get_signature() == &subtract.get_signature() );
}

TEST_CASE(
    "a declared operation should reject operands its rule forbids",
    "[trivial_operation]"
)
{
    const stub_add_operation op;

    const std::vector<operand_specification> mismatched = {
        operand_specification({ 4 }, numerical_type::float32),
        operand_specification({ 4 }, numerical_type::int32)
    };
    std::vector<operand_specification> outputs(1);

    CHECK_THROWS_AS(
        op.get_signature().deduce(
            op.get_descriptor(),
            make_span(outputs),
            make_span(mismatched)
        ),
        std::invalid_argument
    );
}

TEST_CASE(
    "an operation with values its signature does not read should keep its "
    "declaration",
    "[trivial_operation]"
)
{
    const stub_fill_operation op(scalar_value(3.0F));

    CHECK( op.get_name() == "stub_fill" );
    CHECK( op.get_arity() == operation_arity(1, 0) );
    CHECK( op.get_fill_value().get<float32_t>() == 3.0F );

    // With no input to fix it, the element type is left for the user
    // supplied output to determine.
    std::vector<operand_specification> outputs(1);
    op.get_signature().deduce(
        op.get_descriptor(),
        make_span(outputs),
        span<const operand_specification>()
    );
    CHECK( outputs[0].get_data_type() == numerical_type::unknown );
    CHECK( outputs[0].get_shape().empty() );
}
