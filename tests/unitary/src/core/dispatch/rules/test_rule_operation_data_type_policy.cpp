// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <xmipp4/core/dispatch/rules/rule_operation_data_type_policy.hpp>

#include <xmipp4/core/dispatch/operation_descriptor.hpp>
#include <xmipp4/core/platform/constexpr.hpp>
#include <xmipp4/core/dispatch/rules/operand_type_rule_engine.hpp>
#include <xmipp4/ops/rules/operand_type_rules.hpp>

#include <stdexcept>
#include <string>
#include <vector>

using namespace xmipp4;

namespace
{

using type_vector = std::vector<numerical_type>;

/**
 * @brief A description for the rules under test to name in diagnostics.
 *
 * The operand names are generic because these tests exercise the rules
 * rather than any particular operation.
 */
const operation_descriptor& rule_descriptor()
{
	static REX_CONST_CONSTEXPR auto outputs =
		make_operand_names("result", "second_result");
	static REX_CONST_CONSTEXPR auto inputs =
		make_operand_names("left", "right", "third");
	static const operation_descriptor instance =
		make_operation_descriptor("xmipp4.test", "probe", outputs, inputs);
	return instance;
}

type_vector all_concrete_types()
{
    type_vector result;
    const auto count = static_cast<int>(numerical_type::count);
    for (int i = 0; i < count; ++i)
    {
        result.push_back(static_cast<numerical_type>(i));
    }
    return result;
}

struct deduce_outcome
{
    bool rejected = false;
    type_vector outputs;
};

bool operator==(const deduce_outcome &lhs, const deduce_outcome &rhs)
{
    if (lhs.rejected != rhs.rejected)
    {
        return false;
    }
    return lhs.rejected || lhs.outputs == rhs.outputs;
}

deduce_outcome run_deduce(
    const operation_data_type_policy &policy,
    const type_vector &inputs,
    std::size_t output_count
)
{
    deduce_outcome outcome;
    outcome.outputs.assign(output_count, numerical_type::unknown);

    try
    {
        policy.deduce(rule_descriptor(), make_span(outcome.outputs), make_span(inputs));
    }
    catch (const std::invalid_argument&)
    {
        outcome.rejected = true;
    }

    return outcome;
}

bool run_accept(
    const operation_data_type_policy &policy,
    const type_vector &user_outputs,
    const type_vector &inputs
)
{
    // The canonical types are whatever deduce produced, which is how the
    // dispatcher calls accept.
    const auto canonical = run_deduce(policy, inputs, user_outputs.size());
    if (canonical.rejected)
    {
        return true;
    }

    try
    {
        policy.accept(
            rule_descriptor(),
            make_span(user_outputs),
            make_span(canonical.outputs),
            make_span(inputs)
        );
    }
    catch (const std::invalid_argument&)
    {
        return true;
    }

    return false;
}

} // anonymous namespace

TEST_CASE(
    "binary_homogeneous_rule should require one shared element type",
    "[rule_operation_data_type_policy]"
)
{
    const auto &rule =
        rule_data_type_policy<ops::binary_homogeneous_rule<>>::get();

    for (const auto left : all_concrete_types())
    {
        for (const auto right : all_concrete_types())
        {
            const type_vector inputs = { left, right };
            const auto outcome = run_deduce(rule, inputs, 1);

            INFO( "deduce(" << left << ", " << right << ")" );
            if (left == right)
            {
                CHECK( !outcome.rejected );
                CHECK( outcome.outputs == type_vector{ left } );
            }
            else
            {
                CHECK( outcome.rejected );
            }

            // The inputs fix the output, so only the matching one is
            // admitted.
            INFO( "accept(" << left << ")" );
            CHECK( run_accept(rule, type_vector{ left }, inputs) ==
                   (left != right) );
        }
    }
}

TEST_CASE(
    "unary_real_of_rule should type the abs operation",
    "[rule_operation_data_type_policy]"
)
{
    const auto &rule = rule_data_type_policy<
        ops::unary_real_of_rule<arithmetic_type_domain>
    >::get();

    for (const auto input : all_concrete_types())
    {
        const type_vector inputs = { input };
        const auto outcome = run_deduce(rule, inputs, 1);

        INFO( "deduce(" << input << ")" );
        if (arithmetic_type_domain::get().contains(input))
        {
            // Complex inputs lose their imaginary part; everything else is
            // left alone.
            CHECK( !outcome.rejected );
            CHECK( outcome.outputs == type_vector{ make_real(input) } );
        }
        else
        {
            // Booleans and characters have no magnitude to take.
            CHECK( outcome.rejected );
        }
    }

    // The output is fixed by the input, so a user supplied output of any
    // other type is refused.
    CHECK( !run_accept(
        rule,
        type_vector{ numerical_type::float32 },
        type_vector{ numerical_type::complex_float32 }
    ) );
    CHECK( run_accept(
        rule,
        type_vector{ numerical_type::complex_float32 },
        type_vector{ numerical_type::complex_float32 }
    ) );
}

TEST_CASE(
    "converting_rule should type the copy operation",
    "[rule_operation_data_type_policy]"
)
{
    const auto &rule = rule_data_type_policy<ops::converting_rule<>>::get();

    for (const auto input : all_concrete_types())
    {
        const type_vector inputs = { input };

        // With no output supplied, the destination adopts the source type.
        const auto outcome = run_deduce(rule, inputs, 1);
        INFO( "deduce(" << input << ")" );
        CHECK( !outcome.rejected );
        CHECK( outcome.outputs == type_vector{ input } );

        // With one supplied, any concrete type is admitted: that is what
        // makes a converting copy possible without a separate operation.
        for (const auto output : all_concrete_types())
        {
            INFO( "accept(" << output << ", " << input << ")" );
            CHECK( !run_accept(rule, type_vector{ output }, inputs) );
        }
    }
}

TEST_CASE(
    "nullary_free_rule should leave the element type to the output",
    "[rule_operation_data_type_policy]"
)
{
    const auto &rule =
        rule_data_type_policy<ops::nullary_free_rule<>>::get();

    const type_vector no_inputs;

    // With no input to fix it, the element type stays undetermined until
    // the caller supplies an output carrying one.
    const auto outcome = run_deduce(rule, no_inputs, 1);
    CHECK( !outcome.rejected );
    CHECK( outcome.outputs == type_vector{ numerical_type::unknown } );

    for (const auto output : all_concrete_types())
    {
        INFO( "accept(" << output << ")" );
        CHECK( !run_accept(rule, type_vector{ output }, no_inputs) );
    }
}

TEST_CASE(
    "a rule should reject inputs outside its declared domain",
    "[rule_operation_data_type_policy]"
)
{
    const auto &rule = rule_data_type_policy<
        ops::binary_homogeneous_rule<real_arithmetic_type_domain>
    >::get();

    type_vector outputs(1, numerical_type::unknown);

    const type_vector accepted = {
        numerical_type::int32,
        numerical_type::int32
    };
    CHECK_NOTHROW(
        rule.deduce(rule_descriptor(), make_span(outputs), make_span(accepted))
    );

    // This is the drift the declarative rules exist to remove: a domain the
    // operation declares once, rather than a backend predicate silently
    // narrowing what the policy already accepted.
    const type_vector rejected = {
        numerical_type::complex_float32,
        numerical_type::complex_float32
    };
    CHECK_THROWS_AS(
        rule.deduce(rule_descriptor(), make_span(outputs), make_span(rejected)),
        std::invalid_argument
    );

    const type_vector booleans = {
        numerical_type::boolean,
        numerical_type::boolean
    };
    CHECK_THROWS_AS(
        rule.deduce(rule_descriptor(), make_span(outputs), make_span(booleans)),
        std::invalid_argument
    );
}

TEST_CASE(
    "a rule should produce a boolean output for a predicate",
    "[rule_operation_data_type_policy]"
)
{
    const auto &rule =
        rule_data_type_policy<ops::binary_predicate_rule<>>::get();

    type_vector outputs(1, numerical_type::unknown);
    const type_vector inputs = {
        numerical_type::float64,
        numerical_type::float64
    };

    rule.deduce(rule_descriptor(), make_span(outputs), make_span(inputs));
    CHECK( outputs[0] == numerical_type::boolean );
}

TEST_CASE(
    "a rule should name the offending operand and the expected type",
    "[rule_operation_data_type_policy]"
)
{
    const auto &rule =
        rule_data_type_policy<ops::binary_homogeneous_rule<>>::get();

    type_vector outputs(1, numerical_type::unknown);
    const type_vector mismatched = {
        numerical_type::float32,
        numerical_type::float64
    };

    try
    {
        rule.deduce(rule_descriptor(), make_span(outputs), make_span(mismatched));
        FAIL( "expected the mismatching operands to be rejected" );
    }
    catch (const std::invalid_argument &error)
    {
        const std::string message = error.what();
        using Catch::Matchers::ContainsSubstring;
        // Naming the operand is the whole point of the operand names: an
        // index alone is not much help behind a Python front end.
        CHECK_THAT( message, ContainsSubstring("xmipp4.test.probe") );
        CHECK_THAT( message, ContainsSubstring("input operand 'right'") );
        CHECK_THAT( message, ContainsSubstring("float32") );
        CHECK_THAT( message, ContainsSubstring("float64") );
    }
}

TEST_CASE(
    "a rule should report the admissible types when a domain is violated",
    "[rule_operation_data_type_policy]"
)
{
    const auto &rule = rule_data_type_policy<
        ops::unary_homogeneous_rule<floating_point_type_domain>
    >::get();

    type_vector outputs(1, numerical_type::unknown);
    const type_vector inputs = { numerical_type::int32 };

    try
    {
        rule.deduce(rule_descriptor(), make_span(outputs), make_span(inputs));
        FAIL( "expected the input to be rejected" );
    }
    catch (const std::invalid_argument &error)
    {
        const std::string message = error.what();
        using Catch::Matchers::ContainsSubstring;
        CHECK_THAT( message, ContainsSubstring("int32") );
        CHECK_THAT( message, ContainsSubstring("float16, float32, float64") );
    }
}

TEST_CASE(
    "an operation supplied pivot should fix the output type",
    "[rule_operation_data_type_policy]"
)
{
    // The astype shape: the output type is a parameter of the operation
    // instance rather than a property of the operands, so the policy is
    // owned by the operation instead of shared.
    using astype_rule = operand_type_rule<
        type_list<pivot_from_operation<0>, pivot_from_input<0>>,
        type_list<slot_same_as<0>>,
        type_list<slot_same_as<1>>
    >;

    const rule_operation_data_type_policy policy(
        pivot_descriptor_table<astype_rule::pivot_list>::get(),
        slot_descriptor_table<astype_rule::output_slot_list>::get(),
        slot_descriptor_table<astype_rule::input_slot_list>::get(),
        { numerical_type::float64 }
    );

    type_vector outputs(1, numerical_type::unknown);
    const type_vector inputs = { numerical_type::int32 };

    policy.deduce(rule_descriptor(), make_span(outputs), make_span(inputs));
    CHECK( outputs[0] == numerical_type::float64 );
}
