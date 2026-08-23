// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/dispatch/rules/rule_operand_types.hpp>

#include <rex/core/dispatch/operation_descriptor.hpp>
#include <rex/core/platform/constexpr.hpp>

#include <rex/core/dispatch/rules/operand_type_descriptor.hpp>
#include <rex/core/dispatch/rules/rule_operation_data_type_policy.hpp>
#include <rex/core/numerical/numerical_type_dispatch.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

#include "safe_numerical_type_of.hpp"

#include <complex>
#include <type_traits>
#include <vector>

using namespace rex;
using xmipp4::test::safe_numerical_type_of;

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

template <typename List>
struct to_numerical_types;

template <typename... Ts>
struct to_numerical_types<type_list<Ts...>>
{
	static type_vector get()
	{
		return type_vector{ safe_numerical_type_of<Ts>::value... };
	}
};

/**
 * @brief Assert that both interpreters of a rule produce the same types.
 *
 * The runtime policy validates operand types and the backend dispatchers
 * reify them into template arguments. If the two ever disagreed, a program
 * would be built for element types the policy never approved. Only single
 * pivot rules are covered, which is every rule a backend can dispatch from
 * one operand type.
 *
 * @tparam Rule The rule under test.
 */
template <typename Rule>
void check_rule_agreement()
{
	static_assert(
		Rule::pivot_count == 1,
		"This check drives the rule from a single pivot value."
	);

	const auto &policy = rule_data_type_policy<Rule>::get();
	const auto pivots =
		pivot_descriptor_table<typename Rule::pivot_list>::get();
	const auto &domain = pivots[0].get_domain();

	const auto count = static_cast<int>(numerical_type::count);
	for (int i = 0; i < count; ++i)
	{
		const auto pivot = static_cast<numerical_type>(i);
		if (!domain.contains(pivot))
		{
			continue;
		}

		// The type level half, as a backend would instantiate it.
		const auto reified = dispatch_numerical_types(
			[] (auto tag)
			{
				using pivot_types = type_list<typename decltype(tag)::type>;

				return std::make_pair(
					to_numerical_types<rule_operand_types_t<
						typename Rule::output_slot_list,
						pivot_types
					>>::get(),
					to_numerical_types<rule_operand_types_t<
						typename Rule::input_slot_list,
						pivot_types
					>>::get()
				);
			},
			pivot
		);

		// The value level half, as the dispatcher would run it.
		type_vector deduced(Rule::output_count, numerical_type::unknown);
		policy.deduce(
			rule_descriptor(),
			make_span(deduced),
			make_span(reified.second)
		);

		INFO( "pivot " << pivot );
		CHECK( deduced == reified.first );
	}
}

} // anonymous namespace

TEST_CASE(
    "rule_operand_types should apply each slot transform to its pivot",
    "[rule_operand_types]"
)
{
    using rule = ops::unary_real_of_rule<>;

    STATIC_REQUIRE( std::is_same<
        rule_operand_types_t<
            rule::output_slot_list,
            type_list<std::complex<float32_t>>
        >,
        type_list<float32_t>
    >::value );

    STATIC_REQUIRE( std::is_same<
        rule_operand_types_t<
            rule::input_slot_list,
            type_list<std::complex<float32_t>>
        >,
        type_list<std::complex<float32_t>>
    >::value );
}

TEST_CASE(
    "rule_operand_types should repeat the pivot for a homogeneous rule",
    "[rule_operand_types]"
)
{
    using rule = ops::binary_homogeneous_rule<>;

    STATIC_REQUIRE( std::is_same<
        rule_operand_types_t<rule::input_slot_list, type_list<float64_t>>,
        type_list<float64_t, float64_t>
    >::value );
}

TEST_CASE(
    "rule_operand_types should yield an empty list for a nullary rule",
    "[rule_operand_types]"
)
{
    using rule = ops::nullary_free_rule<>;

    STATIC_REQUIRE( std::is_same<
        rule_operand_types_t<rule::input_slot_list, type_list<float32_t>>,
        type_list<>
    >::value );
}

TEST_CASE(
    "rule_operand_types should honour a fixed slot",
    "[rule_operand_types]"
)
{
    using rule = ops::binary_predicate_rule<>;

    STATIC_REQUIRE( std::is_same<
        rule_operand_types_t<rule::output_slot_list, type_list<float32_t>>,
        type_list<bool>
    >::value );
}

TEST_CASE(
    "rule_operand_types should honour a fixed slot on an input",
    "[rule_operand_types]"
)
{
    // A mask is an operand of a fixed type rather than a parameter, so the
    // fixed transform has to work on the input side as well as the output
    // side. Nothing in the slot machinery distinguishes the two.
    using rule = ops::masked_unary_homogeneous_rule<>;

    STATIC_REQUIRE( std::is_same<
        rule_operand_types_t<rule::input_slot_list, type_list<float32_t>>,
        type_list<float32_t, bool>
    >::value );
}

TEST_CASE(
    "rule_operand_types should follow a pivot taken from a later input",
    "[rule_operand_types]"
)
{
    // The condition of a selection is boolean whatever the selected type is,
    // so the pivot cannot come from input 0. Pivot indices and operand
    // indices are separate spaces, and this is where that matters.
    using rule = ops::selection_rule<>;

    STATIC_REQUIRE( rule::pivot_count == 1 );

    STATIC_REQUIRE( std::is_same<
        rule_operand_types_t<rule::input_slot_list, type_list<std::int32_t>>,
        type_list<bool, std::int32_t, std::int32_t>
    >::value );

    STATIC_REQUIRE( std::is_same<
        rule_operand_types_t<rule::output_slot_list, type_list<std::int32_t>>,
        type_list<std::int32_t>
    >::value );
}

TEST_CASE(
    "rule_operand_types should give every output of a pair rule the "
    "operand type",
    "[rule_operand_types]"
)
{
    using rule = ops::binary_homogeneous_pair_rule<>;

    STATIC_REQUIRE( rule::output_count == 2 );

    STATIC_REQUIRE( std::is_same<
        rule_operand_types_t<rule::output_slot_list, type_list<std::int64_t>>,
        type_list<std::int64_t, std::int64_t>
    >::value );
}

TEST_CASE(
    "rule_operand_types should keep the pivots of a heterogeneous rule "
    "apart",
    "[rule_operand_types]"
)
{
    // Two independent pivots, so the second operand does not follow the
    // first. This is the only rule shape a backend cannot dispatch from a
    // single operand type.
    using rule = ops::binary_heterogeneous_rule<>;

    STATIC_REQUIRE( rule::pivot_count == 2 );

    STATIC_REQUIRE( std::is_same<
        rule_operand_types_t<
            rule::input_slot_list,
            type_list<float32_t, std::int32_t>
        >,
        type_list<float32_t, std::int32_t>
    >::value );

    STATIC_REQUIRE( std::is_same<
        rule_operand_types_t<
            rule::output_slot_list,
            type_list<float32_t, std::int32_t>
        >,
        type_list<float32_t>
    >::value );
}

TEST_CASE(
    "both interpreters of a rule should agree on every operand type",
    "[rule_operand_types]"
)
{
    check_rule_agreement<ops::unary_homogeneous_rule<>>();
    check_rule_agreement<ops::binary_homogeneous_rule<>>();
    check_rule_agreement<ops::ternary_homogeneous_rule<>>();
    check_rule_agreement<ops::unary_real_of_rule<>>();
    check_rule_agreement<ops::unary_complex_of_rule<>>();
    check_rule_agreement<ops::unary_inexact_of_rule<>>();
    check_rule_agreement<ops::unary_predicate_rule<>>();
    check_rule_agreement<ops::binary_predicate_rule<>>();
    check_rule_agreement<
        ops::unary_fixed_output_rule<numerical_type::int64>
    >();
    check_rule_agreement<ops::binary_homogeneous_pair_rule<>>();
    check_rule_agreement<ops::masked_unary_homogeneous_rule<>>();
    check_rule_agreement<ops::masked_binary_homogeneous_rule<>>();
    check_rule_agreement<ops::selection_rule<>>();
    check_rule_agreement<
        ops::binary_homogeneous_rule<real_arithmetic_type_domain>
    >();

    // binary_heterogeneous_rule is deliberately absent: it has two pivots,
    // and this check drives a rule from a single pivot value. Its type level
    // half is covered by the static assertions above.
}
