// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <rex/core/dispatch/operation_cast.hpp>

#include <rex/core/dispatch/basic_operation.hpp>
#include <rex/ops/ops_component.hpp>
#include <rex/ops/policies/elementwise_operation_shape_policy.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

#include <stdexcept>
#include <string>

using namespace rex;

namespace
{

REXLIB_DECLARE_OPERATION_TRAITS(
	cast_probe,
	ops::ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	ops::elementwise_operation_shape_policy,
	ops::unary_homogeneous_rule<>
);

// Not final, so that the substitutability of a specialised operation can be
// exercised.
class cast_probe_operation
	: public trivial_operation<cast_probe_operation, cast_probe_operation_traits>
{
public:
	virtual int get_marker() const noexcept
	{
		return 1;
	}
};

class refined_cast_probe_operation final
	: public cast_probe_operation
{
public:
	int get_marker() const noexcept override
	{
		return 2;
	}
};

REXLIB_DECLARE_OPERATION(
	other_probe,
	ops::ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	ops::elementwise_operation_shape_policy,
	ops::unary_homogeneous_rule<>
);

} // anonymous namespace

TEST_CASE(
    "operation_cast should downcast a matching operation",
    "[operation_cast]"
)
{
    const cast_probe_operation concrete;
    const operation &op = concrete;

    CHECK( operation_cast<cast_probe_operation>(op).get_marker() == 1 );
    CHECK( &operation_cast<cast_probe_operation>(op) == &concrete );
}

TEST_CASE(
    "operation_cast should reject an unrelated operation",
    "[operation_cast]"
)
{
    const other_probe_operation concrete;
    const operation &op = concrete;

    CHECK_THROWS_AS(
        operation_cast<cast_probe_operation>(op),
        std::invalid_argument
    );
}

TEST_CASE(
    "operation_cast should name both operations when it rejects one",
    "[operation_cast]"
)
{
    const other_probe_operation concrete;
    const operation &op = concrete;

    try
    {
        operation_cast<cast_probe_operation>(op);
        FAIL( "expected the unrelated operation to be rejected" );
    }
    catch (const std::invalid_argument &error)
    {
        const std::string message = error.what();
        using Catch::Matchers::ContainsSubstring;
        CHECK_THAT( message, ContainsSubstring("cast_probe") );
        CHECK_THAT( message, ContainsSubstring("other_probe") );
    }
}

TEST_CASE(
    "an operation should report the identity of the operation it refines",
    "[operation_cast]"
)
{
    const refined_cast_probe_operation refined;
    const operation &op = refined;

    // The identity binds an operation to the builders implementing it. A
    // refinement that behaves as its base must therefore keep the base's
    // identity, or it would silently match no builder at all.
    CHECK( op.get_id() == operation_id::of<cast_probe_operation>() );

    // And the downcast must follow, reaching the refined behaviour through
    // the base the builder asked for.
    CHECK( operation_cast<cast_probe_operation>(op).get_marker() == 2 );
}
