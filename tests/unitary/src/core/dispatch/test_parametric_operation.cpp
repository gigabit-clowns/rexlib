// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/dispatch/parametric_operation.hpp>

#include <rex/ops/ops_component.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

#include <cstddef>
#include <type_traits>
#include <vector>

using namespace rex;

namespace
{

// A shape policy whose deduction depends on what it was constructed with,
// standing in for a reduction's axes. A stateless policy hands out a
// singleton; this one cannot, which is the whole reason
// parametric_operation exists.
class stub_scaling_shape_policy final
	: public operation_shape_policy
{
public:
	explicit stub_scaling_shape_policy(std::size_t factor = 1) noexcept
		: m_factor(factor)
	{
	}

	std::size_t get_factor() const noexcept
	{
		return m_factor;
	}

	void deduce(
		const operation_descriptor & /*descriptor*/,
		span<shape_type> canonical_output_shapes,
		span<const shape_type> input_shapes
	) const override
	{
		for (auto &shape : canonical_output_shapes)
		{
			shape = input_shapes.empty() ? shape_type() : input_shapes[0];
			for (auto &extent : shape)
			{
				extent *= m_factor;
			}
		}
	}

private:
	std::size_t m_factor;
};

REX_DECLARE_PARAMETRIC_OPERATION(
	stub_scale,
	ops::ops_component,
	REX_OPERANDS("result"),
	REX_OPERANDS("value"),
	stub_scaling_shape_policy,
	ops::unary_homogeneous_rule<>
);

} // anonymous namespace

TEST_CASE(
    "a parametric operation should own its shape policy per instance",
    "[parametric_operation]"
)
{
    const stub_scale_operation doubling(2);
    const stub_scale_operation tripling(3);

    // Two instances of one operation, deducing different shapes. A shared
    // policy could not do this, which is what separates parametric_operation
    // from trivial_operation.
    CHECK( &doubling.get_operation_shape_policy() !=
           &tripling.get_operation_shape_policy() );

    const std::vector<operation_shape_policy::shape_type> inputs = {
        { 2, 3 }
    };
    std::vector<operation_shape_policy::shape_type> outputs(1);

    doubling.get_operation_shape_policy().deduce(
        doubling.get_descriptor(),
        make_span(outputs),
        make_span(inputs)
    );
    CHECK( outputs[0] == operation_shape_policy::shape_type{ 4, 6 } );

    tripling.get_operation_shape_policy().deduce(
        tripling.get_descriptor(),
        make_span(outputs),
        make_span(inputs)
    );
    CHECK( outputs[0] == operation_shape_policy::shape_type{ 6, 9 } );
}

TEST_CASE(
    "a parametric operation should expose its policy with its own type",
    "[parametric_operation]"
)
{
    const stub_scale_operation op(4);

    // A program builder is instantiated on the operation type, so it reads
    // the parameters back through this accessor rather than downcasting the
    // erased policy it would otherwise get.
    STATIC_REQUIRE( std::is_same<
        decltype(op.get_shape_policy()),
        const stub_scaling_shape_policy&
    >::value );

    CHECK( op.get_shape_policy().get_factor() == 4 );
    CHECK( &op.get_shape_policy() == &op.get_operation_shape_policy() );
}

TEST_CASE(
    "a parametric operation should default construct its shape policy",
    "[parametric_operation]"
)
{
    // The constructor is variadic, so it has to keep working with no
    // argument at all when the policy is default constructible.
    const stub_scale_operation op;
    CHECK( op.get_shape_policy().get_factor() == 1 );
}

TEST_CASE(
    "a parametric operation should keep everything a declared one has",
    "[parametric_operation]"
)
{
    const stub_scale_operation op(2);

    // Identity, description and typing come from basic_operation just as
    // they do for a trivially declared operation, so the program builders
    // reading them need no special case.
    CHECK( op.get_name() == "stub_scale" );
    CHECK( op.get_arity() == operation_arity::unary() );
    CHECK( op.get_id() == operation_id::of<stub_scale_operation>() );
    STATIC_REQUIRE( stub_scale_operation::output_operand_count == 1 );
    STATIC_REQUIRE( stub_scale_operation::input_operand_count == 1 );
    STATIC_REQUIRE( std::is_final<stub_scale_operation>::value );
    STATIC_REQUIRE(
        std::is_base_of<operation, stub_scale_operation>::value
    );

    // The data type policy stays the shared one derived from the rule: a
    // shape parameter does not change the typing.
    const stub_scale_operation other(7);
    CHECK( &op.get_operation_data_type_policy() ==
           &other.get_operation_data_type_policy() );

    // The description is shared too, being static.
    CHECK( &op.get_descriptor() == &other.get_descriptor() );
}
