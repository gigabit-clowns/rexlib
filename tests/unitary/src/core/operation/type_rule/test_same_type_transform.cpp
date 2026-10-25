// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/same_type_transform.hpp>

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain.hpp>

#include "fixtures/type_transform_consistency.hpp"

#include <string>
#include <type_traits>

using namespace rexlib;

TEST_CASE(
	"same_type_transform should agree on both of its halves",
	"[same_type_transform]"
)
{
	test::check_type_transform_consistency<same_type_transform>();
}

TEST_CASE(
	"same_type_transform should map every type onto itself",
	"[same_type_transform]"
)
{
	CHECK( same_type_transform::domain_type::get() ==
	       numerical_type_domain::all() );
	CHECK( same_type_transform::apply(numerical_type::int8) ==
	       numerical_type::int8 );
	CHECK( same_type_transform::apply(numerical_type::complex_float64) ==
	       numerical_type::complex_float64 );

	STATIC_REQUIRE( std::is_same<
		same_type_transform::apply_type<float32_t>::type,
		float32_t
	>::value );
}

TEST_CASE(
	"same_type_transform should name its relation",
	"[same_type_transform]"
)
{
	CHECK( std::string(same_type_transform::get_relation()) ==
	       "the same type as" );
}
