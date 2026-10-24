// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/shape_rule/core_dimensions.hpp>

#include <rexlib/core/operation/shape_rule/positive_extent_domain.hpp>

#include "fixtures/doubled_extent_transform.hpp"

#include <type_traits>

using namespace rexlib;

TEST_CASE(
	"core_dimensions without slots should declare no core axis",
	"[core_dimensions]"
)
{
	using core = core_dimensions<>;
	STATIC_CHECK_FALSE( core::has_run );
	STATIC_CHECK( core::axis_count == 0 );
	STATIC_CHECK( std::is_same<core::slot_list, type_list<>>::value );
}

TEST_CASE(
	"core_dimensions should keep its slots in order",
	"[core_dimensions]"
)
{
	using core = core_dimensions<axis_same_as<1>, test::axis_doubled_of<0>>;
	STATIC_CHECK_FALSE( core::has_run );
	STATIC_CHECK( core::axis_count == 2 );
	STATIC_CHECK(
		std::is_same<
			core::slot_list,
			type_list<axis_same_as<1>, test::axis_doubled_of<0>>
		>::value
	);
}

TEST_CASE(
	"core_dimensions should take a leading run apart from its slots",
	"[core_dimensions]"
)
{
	using core = core_dimensions<
		axis_run<positive_extent_domain>,
		axis_same_as<0>
	>;
	STATIC_CHECK( core::has_run );
	STATIC_CHECK( core::axis_count == 1 );
	STATIC_CHECK(
		std::is_same<core::slot_list, type_list<axis_same_as<0>>>::value
	);
	STATIC_CHECK(
		std::is_same<
			core::run_domain_type,
			positive_extent_domain
		>::value
	);
}

TEST_CASE(
	"core_dimensions with a run alone should declare no further axis",
	"[core_dimensions]"
)
{
	using core = core_dimensions<axis_run<>>;
	STATIC_CHECK( core::has_run );
	STATIC_CHECK( core::axis_count == 0 );
	STATIC_CHECK(
		std::is_same<core::run_domain_type, any_extent_domain>::value
	);
}
