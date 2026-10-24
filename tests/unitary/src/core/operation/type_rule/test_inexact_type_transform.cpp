// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/inexact_type_transform.hpp>

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain.hpp>

#include "fixtures/type_transform_consistency.hpp"

#include <complex>
#include <cstdint>
#include <string>
#include <type_traits>

using namespace rexlib;

TEST_CASE(
	"inexact_type_transform should agree on both of its halves",
	"[inexact_type_transform]"
)
{
	test::check_type_transform_consistency<inexact_type_transform>();
}

TEST_CASE(
	"inexact_type_transform should exclude characters only",
	"[inexact_type_transform]"
)
{
	const auto &domain = inexact_type_transform::domain_type::get();
	CHECK( !domain.contains(numerical_type::char8) );
	CHECK( domain.contains(numerical_type::boolean) );
	CHECK( domain.contains(numerical_type::int32) );
	CHECK( domain.contains(numerical_type::float32) );
	CHECK( domain.contains(numerical_type::complex_float32) );
}

TEST_CASE(
	"inexact_type_transform should widen exact types and keep inexact ones",
	"[inexact_type_transform]"
)
{
	CHECK( inexact_type_transform::apply(numerical_type::int32) ==
	       numerical_type::float64 );
	CHECK( inexact_type_transform::apply(numerical_type::uint64) ==
	       numerical_type::float64 );
	CHECK( inexact_type_transform::apply(numerical_type::boolean) ==
	       numerical_type::float64 );
	CHECK( inexact_type_transform::apply(numerical_type::float16) ==
	       numerical_type::float16 );
	CHECK( inexact_type_transform::apply(numerical_type::float32) ==
	       numerical_type::float32 );
	CHECK( inexact_type_transform::apply(numerical_type::complex_float32) ==
	       numerical_type::complex_float32 );

	STATIC_REQUIRE( std::is_same<
		inexact_type_transform::apply_type<std::int32_t>::type,
		float64_t
	>::value );
	STATIC_REQUIRE( std::is_same<
		inexact_type_transform::apply_type<float32_t>::type,
		float32_t
	>::value );
	STATIC_REQUIRE( std::is_same<
		inexact_type_transform::apply_type<std::complex<float32_t>>::type,
		std::complex<float32_t>
	>::value );
}

TEST_CASE(
	"inexact_type_transform should name its relation",
	"[inexact_type_transform]"
)
{
	CHECK( std::string(inexact_type_transform::get_relation()) ==
	       "the inexact counterpart of" );
}
