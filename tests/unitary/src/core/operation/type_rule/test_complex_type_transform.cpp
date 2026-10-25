// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/complex_type_transform.hpp>

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain.hpp>

#include "fixtures/type_transform_consistency.hpp"

#include <complex>
#include <string>
#include <type_traits>

using namespace rexlib;

TEST_CASE(
	"complex_type_transform should agree on both of its halves",
	"[complex_type_transform]"
)
{
	test::check_type_transform_consistency<complex_type_transform>();
}

TEST_CASE(
	"complex_type_transform should exclude the types it cannot map",
	"[complex_type_transform]"
)
{
	const auto &domain = complex_type_transform::domain_type::get();
	CHECK( !domain.contains(numerical_type::int32) );
	CHECK( !domain.contains(numerical_type::boolean) );
	CHECK( domain.contains(numerical_type::float32) );
	CHECK( domain.contains(numerical_type::complex_float32) );
}

TEST_CASE(
	"complex_type_transform should map real types onto their complex "
	"counterpart",
	"[complex_type_transform]"
)
{
	CHECK( complex_type_transform::apply(numerical_type::float32) ==
	       numerical_type::complex_float32 );
	CHECK( complex_type_transform::apply(numerical_type::complex_float64) ==
	       numerical_type::complex_float64 );

	STATIC_REQUIRE( std::is_same<
		complex_type_transform::apply_type<float64_t>::type,
		std::complex<float64_t>
	>::value );
	STATIC_REQUIRE( std::is_same<
		complex_type_transform::apply_type<std::complex<float32_t>>::type,
		std::complex<float32_t>
	>::value );
}

TEST_CASE(
	"complex_type_transform should name its relation",
	"[complex_type_transform]"
)
{
	CHECK( std::string(complex_type_transform::get_relation()) ==
	       "the complex counterpart of" );
}
