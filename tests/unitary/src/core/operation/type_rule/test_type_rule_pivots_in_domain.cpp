// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/type_rule_pivots_in_domain.hpp>

#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>
#include <rexlib/core/numerical/numerical_type_traits.hpp>
#include <rexlib/core/operation/type_rule/type_pivot.hpp>

#include <complex>
#include <cstdint>

using namespace rexlib;

namespace
{

using pivots = type_list<
	pivot_from_input<0, floating_point_type_domain>,
	pivot_from_input<1, integral_type_domain>
>;

} // anonymous namespace

TEST_CASE(
	"type_rule_pivots_in_domain should accept a rule without pivots",
	"[type_rule_pivots_in_domain]"
)
{
	STATIC_REQUIRE(
		type_rule_pivots_in_domain<type_list<>, type_list<>>::value
	);
}

TEST_CASE(
	"type_rule_pivots_in_domain should accept types inside every domain",
	"[type_rule_pivots_in_domain]"
)
{
	STATIC_REQUIRE(
		type_rule_pivots_in_domain<
			pivots,
			type_list<float32_t, std::int16_t>
		>::value
	);
}

TEST_CASE(
	"type_rule_pivots_in_domain should reject a type outside its domain",
	"[type_rule_pivots_in_domain]"
)
{
	STATIC_REQUIRE_FALSE(
		type_rule_pivots_in_domain<
			pivots,
			type_list<std::int16_t, std::int16_t>
		>::value
	);
	STATIC_REQUIRE_FALSE(
		type_rule_pivots_in_domain<
			pivots,
			type_list<float32_t, std::complex<float32_t>>
		>::value
	);
}
