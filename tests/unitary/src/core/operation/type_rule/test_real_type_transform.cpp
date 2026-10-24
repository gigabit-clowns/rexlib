// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/real_type_transform.hpp>

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain.hpp>

#include "fixtures/type_transform_consistency.hpp"

#include <complex>
#include <cstdint>
#include <string>
#include <type_traits>

using namespace rexlib;

TEST_CASE(
	"real_type_transform should agree on both of its halves",
	"[real_type_transform]"
)
{
	test::check_type_transform_consistency<real_type_transform>();
}

TEST_CASE(
	"real_type_transform should be defined on every type",
	"[real_type_transform]"
)
{
	CHECK( real_type_transform::domain_type::get() ==
	       numerical_type_domain::all() );
}

TEST_CASE(
	"real_type_transform should map complex types onto their real part",
	"[real_type_transform]"
)
{
	CHECK( real_type_transform::apply(numerical_type::complex_float32) ==
	       numerical_type::float32 );
	CHECK( real_type_transform::apply(numerical_type::int16) ==
	       numerical_type::int16 );

	STATIC_REQUIRE( std::is_same<
		real_type_transform::apply_type<std::complex<float32_t>>::type,
		float32_t
	>::value );
	STATIC_REQUIRE( std::is_same<
		real_type_transform::apply_type<std::int16_t>::type,
		std::int16_t
	>::value );
}

TEST_CASE(
	"real_type_transform should name its relation",
	"[real_type_transform]"
)
{
	CHECK( std::string(real_type_transform::get_relation()) ==
	       "the real counterpart of" );
}
