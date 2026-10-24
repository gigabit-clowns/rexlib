// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/fixed_type_transform.hpp>

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain.hpp>

#include "fixtures/type_transform_consistency.hpp"

#include <cstdint>
#include <string>
#include <type_traits>

using namespace rexlib;

TEST_CASE(
	"fixed_type_transform should agree on both of its halves",
	"[fixed_type_transform]"
)
{
	test::check_type_transform_consistency<
		fixed_type_transform<numerical_type::boolean>
	>();
	test::check_type_transform_consistency<
		fixed_type_transform<numerical_type::float32>
	>();
	test::check_type_transform_consistency<
		fixed_type_transform<numerical_type::complex_float64>
	>();
}

TEST_CASE(
	"fixed_type_transform should yield its type whatever the pivot",
	"[fixed_type_transform]"
)
{
	using transform = fixed_type_transform<numerical_type::boolean>;

	CHECK( transform::domain_type::get() == numerical_type_domain::all() );
	CHECK( transform::apply(numerical_type::int32) ==
	       numerical_type::boolean );
	CHECK( transform::apply(numerical_type::complex_float32) ==
	       numerical_type::boolean );

	STATIC_REQUIRE( std::is_same<
		transform::apply_type<std::int32_t>::type,
		bool
	>::value );
}

TEST_CASE(
	"fixed_type_transform should name its relation",
	"[fixed_type_transform]"
)
{
	CHECK( std::string(
		fixed_type_transform<numerical_type::int8>::get_relation()
	) == "independent of" );
}
