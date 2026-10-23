// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <rexlib/core/dispatch/rules/type_rule_interpreter.hpp>

#include <rexlib/core/operation/operation_descriptor.hpp>
#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/core/dispatch/rules/operand_type_descriptor.hpp>
#include <rexlib/core/dispatch/rules/operand_type_rule.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

using namespace rexlib;

namespace
{

using type_vector = std::vector<numerical_type>;

// Names the data type an operation would supply. The tests pass the values
// themselves, so it is never invoked.
struct test_target_type
{
};

/**
 * @brief A description for the rules under test to name in diagnostics.
 *
 * The operand names are generic because these tests exercise the rules
 * rather than any particular operation.
 */
const operation_descriptor& rule_descriptor()
{
	static REXLIB_CONST_CONSTEXPR auto outputs =
		make_operand_names("result", "second_result");
	static REXLIB_CONST_CONSTEXPR auto inputs =
		make_operand_names("left", "right", "third");
	static const operation_descriptor instance =
		make_operation_descriptor("rexlib.test", "probe", outputs, inputs);
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

template <typename Rule>
type_rule_interpreter interpreter_of()
{
    return type_rule_interpreter(
        pivot_descriptor_table<typename Rule::pivot_list>::get(),
        slot_descriptor_table<typename Rule::output_slot_list>::get(),
        slot_descriptor_table<typename Rule::input_slot_list>::get()
    );
}

std::vector<operand_specification> make_operands(const type_vector &types)
{
    std::vector<operand_specification> result;
    for (const auto type : types)
    {
        result.emplace_back(operand_specification::shape_type(), type);
    }
    return result;
}

void deduce_types(
    const type_rule_interpreter &rule,
    type_vector &outputs,
    const type_vector &inputs,
    const type_vector &operation_pivots = type_vector()
)
{
    auto output_operands = make_operands(outputs);
    const auto input_operands = make_operands(inputs);
    rule.deduce(
        rule_descriptor(),
        make_span(operation_pivots),
        make_span(output_operands),
        make_span(input_operands)
    );
    for (std::size_t i = 0; i < outputs.size(); ++i)
    {
        outputs[i] = output_operands[i].get_data_type();
    }
}

struct deduce_outcome
{
    bool rejected = false;
    type_vector outputs;
};

deduce_outcome run_deduce(
    const type_rule_interpreter &rule,
    const type_vector &inputs,
    std::size_t output_count
)
{
    deduce_outcome outcome;
    outcome.outputs.assign(output_count, numerical_type::unknown);

    try
    {
        deduce_types(rule, outcome.outputs, inputs);
    }
    catch (const std::invalid_argument&)
    {
        outcome.rejected = true;
    }

    return outcome;
}

bool run_accept(
    const type_rule_interpreter &rule,
    const type_vector &user_outputs,
    const type_vector &inputs
)
{
    if (run_deduce(rule, inputs, user_outputs.size()).rejected)
    {
        return true;
    }

    try
    {
        rule.accept(
            rule_descriptor(),
            span<const numerical_type>(),
            make_span(make_operands(user_outputs)),
            make_span(make_operands(inputs))
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
    "[type_rule_interpreter]"
)
{
    const auto rule = interpreter_of<ops::binary_homogeneous_rule<>>();

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
    "[type_rule_interpreter]"
)
{
    const auto rule = interpreter_of<
        ops::unary_real_of_rule<arithmetic_type_domain>
    >();

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
    "[type_rule_interpreter]"
)
{
    const auto rule = interpreter_of<ops::converting_rule<>>();

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
    "[type_rule_interpreter]"
)
{
    const auto rule = interpreter_of<ops::nullary_free_rule<>>();

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
    "[type_rule_interpreter]"
)
{
    const auto rule = interpreter_of<
        ops::binary_homogeneous_rule<real_arithmetic_type_domain>
    >();

    type_vector outputs(1, numerical_type::unknown);

    const type_vector accepted = {
        numerical_type::int32,
        numerical_type::int32
    };
    CHECK_NOTHROW(
        deduce_types(rule, outputs, accepted)
    );

    // This is the drift the declarative rules exist to remove: a domain the
    // operation declares once, rather than a backend predicate silently
    // narrowing what the rule already accepted.
    const type_vector rejected = {
        numerical_type::complex_float32,
        numerical_type::complex_float32
    };
    CHECK_THROWS_AS(
        deduce_types(rule, outputs, rejected),
        std::invalid_argument
    );

    const type_vector booleans = {
        numerical_type::boolean,
        numerical_type::boolean
    };
    CHECK_THROWS_AS(
        deduce_types(rule, outputs, booleans),
        std::invalid_argument
    );
}

TEST_CASE(
    "a rule should produce a boolean output for a predicate",
    "[type_rule_interpreter]"
)
{
    const auto rule = interpreter_of<ops::binary_predicate_rule<>>();

    type_vector outputs(1, numerical_type::unknown);
    const type_vector inputs = {
        numerical_type::float64,
        numerical_type::float64
    };

    deduce_types(rule, outputs, inputs);
    CHECK( outputs[0] == numerical_type::boolean );
}

TEST_CASE(
    "a rule should name the offending operand and the expected type",
    "[type_rule_interpreter]"
)
{
    const auto rule = interpreter_of<ops::binary_homogeneous_rule<>>();

    type_vector outputs(1, numerical_type::unknown);
    const type_vector mismatched = {
        numerical_type::float32,
        numerical_type::float64
    };

    try
    {
        deduce_types(rule, outputs, mismatched);
        FAIL( "expected the mismatching operands to be rejected" );
    }
    catch (const std::invalid_argument &error)
    {
        const std::string message = error.what();
        using Catch::Matchers::ContainsSubstring;
        // Naming the operand is the whole point of the operand names: an
        // index alone is not much help behind a Python front end.
        CHECK_THAT( message, ContainsSubstring("rexlib.test.probe") );
        CHECK_THAT( message, ContainsSubstring("input operand 'right'") );
        CHECK_THAT( message, ContainsSubstring("float32") );
        CHECK_THAT( message, ContainsSubstring("float64") );
    }
}

TEST_CASE(
    "a rule should report the admissible types when a domain is violated",
    "[type_rule_interpreter]"
)
{
    const auto rule = interpreter_of<
        ops::unary_homogeneous_rule<floating_point_type_domain>
    >();

    type_vector outputs(1, numerical_type::unknown);
    const type_vector inputs = { numerical_type::int32 };

    try
    {
        deduce_types(rule, outputs, inputs);
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
    "[type_rule_interpreter]"
)
{
    // The astype shape: the output type is a parameter of the operation
    // instance rather than a property of the operands.
    using astype_rule = operand_type_rule<
        type_list<pivot_from_operation<test_target_type>, pivot_from_input<0>>,
        type_list<slot_same_as<0>>,
        type_list<slot_same_as<1>>
    >;
    const auto rule = interpreter_of<astype_rule>();

    type_vector outputs(1, numerical_type::unknown);
    deduce_types(
        rule,
        outputs,
        type_vector{ numerical_type::int32 },
        type_vector{ numerical_type::float64 }
    );
    CHECK( outputs[0] == numerical_type::float64 );
}

TEST_CASE(
    "operation supplied pivots should take their values in declaration order",
    "[type_rule_interpreter]"
)
{
    using pair_rule = operand_type_rule<
        type_list<
            pivot_from_operation<test_target_type>,
            pivot_from_input<0>,
            pivot_from_operation<test_target_type>
        >,
        type_list<slot_same_as<2>, slot_same_as<0>>,
        type_list<slot_same_as<1>>
    >;
    const auto rule = interpreter_of<pair_rule>();

    type_vector outputs(2, numerical_type::unknown);
    deduce_types(
        rule,
        outputs,
        type_vector{ numerical_type::int8 },
        type_vector{ numerical_type::float32, numerical_type::uint16 }
    );
    CHECK( outputs[0] == numerical_type::uint16 );
    CHECK( outputs[1] == numerical_type::float32 );
}

TEST_CASE(
    "type_rule_interpreter should check the data types the operation "
    "supplies",
    "[type_rule_interpreter]"
)
{
    using astype_rule = operand_type_rule<
        type_list<
            pivot_from_operation<test_target_type, inexact_type_domain>,
            pivot_from_input<0>
        >,
        type_list<slot_same_as<0>>,
        type_list<slot_same_as<1>>
    >;
    const auto rule = interpreter_of<astype_rule>();

    const type_vector admitted = { numerical_type::float32 };
    CHECK_NOTHROW(
        rule.check_instance(rule_descriptor(), make_span(admitted))
    );

    const type_vector outside = { numerical_type::int32 };
    CHECK_THROWS_AS(
        rule.check_instance(rule_descriptor(), make_span(outside)),
        std::invalid_argument
    );

    const type_vector missing;
    CHECK_THROWS_AS(
        rule.check_instance(rule_descriptor(), make_span(missing)),
        std::invalid_argument
    );
}

TEST_CASE(
    "type_rule_interpreter should leave shapes alone",
    "[type_rule_interpreter]"
)
{
    const auto rule = interpreter_of<ops::unary_homogeneous_rule<>>();

    std::vector<operand_specification> outputs = {
        operand_specification({ 3, 4 }, numerical_type::unknown)
    };
    const std::vector<operand_specification> inputs = {
        operand_specification({ 7 }, numerical_type::int16)
    };
    rule.deduce(
        rule_descriptor(),
        span<const numerical_type>(),
        make_span(outputs),
        make_span(inputs)
    );
    CHECK(
        outputs[0] ==
        operand_specification({ 3, 4 }, numerical_type::int16)
    );
}
