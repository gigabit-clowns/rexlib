// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/basic_operation.hpp>

#include <rexlib/core/numerical/scalar_value.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/rules/operand_shape_rules.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

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
    "a declared operation should report its name and arity",
    "[basic_operation]"
)
{
    const stub_add_operation op;

    CHECK( op.get_name() == "stub_add" );
    CHECK( op.get_arity() == operation_arity(1, 2) );
    CHECK( op.get_arity() == operation_arity::binary() );
}

TEST_CASE(
    "a declared operation should expose its operand counts statically",
    "[basic_operation]"
)
{
    // These are what a backend builder needs to size its operand arrays,
    // so they must be usable in a constant expression.
    STATIC_REQUIRE( stub_add_operation::output_operand_count == 1 );
    STATIC_REQUIRE( stub_add_operation::input_operand_count == 2 );
    STATIC_REQUIRE( stub_fill_operation::input_operand_count == 0 );

    std::array<int, stub_add_operation::input_operand_count> usable_as_extent;
    CHECK( usable_as_extent.size() == 2 );
}

TEST_CASE(
    "a declared operation should name its operands",
    "[basic_operation]"
)
{
    const auto &descriptor = stub_add_operation::get_static_descriptor();

    REQUIRE( descriptor.get_input_operand_names().size() == 2 );
    CHECK( std::string(descriptor.get_operand_name(0, true)) == "result" );
    CHECK( std::string(descriptor.get_operand_name(0, false)) == "left" );
    CHECK( std::string(descriptor.get_operand_name(1, false)) == "right" );

    // Naming an operand that does not exist must not be fatal, because the
    // only caller is a diagnostic being built.
    CHECK( descriptor.get_operand_name(2, false) == nullptr );
    CHECK( descriptor.get_operand_name(0, true) != nullptr );
}

TEST_CASE(
    "a declared operation should be qualified by its component",
    "[basic_operation]"
)
{
    std::ostringstream oss;
    oss << stub_add_operation::get_static_descriptor();
    CHECK( oss.str() == "rexlib.ops.stub_add" );
}

TEST_CASE(
    "every instance of an operation should share one descriptor",
    "[basic_operation]"
)
{
    const stub_add_operation first;
    const stub_add_operation second;

    CHECK( &first.get_descriptor() == &second.get_descriptor() );
    CHECK( &first.get_descriptor() ==
           &stub_add_operation::get_static_descriptor() );
}

TEST_CASE(
    "a declared operation should carry the contract of its rules",
    "[basic_operation]"
)
{
    const stub_add_operation op;

    const std::vector<operand_specification> inputs = {
        operand_specification({ 4 }, numerical_type::float32),
        operand_specification({ 1 }, numerical_type::float32)
    };
    std::vector<operand_specification> outputs(1);

    op.get_contract().deduce(
        op.get_descriptor(),
        make_span(outputs),
        make_span(inputs)
    );
    CHECK(
        outputs[0] == operand_specification({ 4 }, numerical_type::float32)
    );
}

TEST_CASE(
    "operations following the same rules should share one contract",
    "[basic_operation]"
)
{
    const stub_add_operation add;
    const stub_subtract_operation subtract;

    CHECK( &add.get_contract() == &subtract.get_contract() );
}

TEST_CASE(
    "a declared operation should reject operands its rule forbids",
    "[basic_operation]"
)
{
    const stub_add_operation op;

    const std::vector<operand_specification> mismatched = {
        operand_specification({ 4 }, numerical_type::float32),
        operand_specification({ 4 }, numerical_type::int32)
    };
    std::vector<operand_specification> outputs(1);

    CHECK_THROWS_AS(
        op.get_contract().deduce(
            op.get_descriptor(),
            make_span(outputs),
            make_span(mismatched)
        ),
        std::invalid_argument
    );
}

TEST_CASE(
    "an operation with values its contract does not read should keep its "
    "declaration",
    "[basic_operation]"
)
{
    const stub_fill_operation op(scalar_value(3.0F));

    CHECK( op.get_name() == "stub_fill" );
    CHECK( op.get_arity() == operation_arity(1, 0) );
    CHECK( op.get_fill_value().get<float32_t>() == 3.0F );

    // With no input to fix it, the element type is left for the user
    // supplied output to determine.
    std::vector<operand_specification> outputs(1);
    op.get_contract().deduce(
        op.get_descriptor(),
        make_span(outputs),
        span<const operand_specification>()
    );
    CHECK( outputs[0].get_data_type() == numerical_type::unknown );
    CHECK( outputs[0].get_shape().empty() );
}

TEST_CASE(
    "a declared operation should be usable through the operation interface",
    "[basic_operation]"
)
{
    const stub_add_operation concrete;
    const operation &op = concrete;

    STATIC_REQUIRE( std::is_base_of<operation, stub_add_operation>::value );
    STATIC_REQUIRE( std::is_final<stub_add_operation>::value );

    CHECK( op.get_name() == "stub_add" );
    CHECK( op.get_arity() == operation_arity::binary() );
    CHECK( op.get_id() == operation_id::of<stub_add_operation>() );
}
