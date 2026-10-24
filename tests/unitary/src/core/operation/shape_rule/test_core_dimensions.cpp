// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/shape_rule/core_dimensions.hpp>

#include <rexlib/core/operation/shape_rule/positive_extent_domain.hpp>

#include <type_traits>

using namespace rexlib;

TEST_CASE(
	"core_dimensions without slots should declare no core axis",
	"[core_dimensions]"
)
{
	using core = core_dimensions<>;
	STATIC_CHECK( core::slot_count == 0 );
	STATIC_CHECK( core::run_count == 0 );
	STATIC_CHECK( core::axis_count == 0 );
}

TEST_CASE(
	"core_dimensions should count a run apart from the other slots",
	"[core_dimensions]"
)
{
	using core = core_dimensions<
		axis_run<positive_extent_domain>,
		axis_same_as<0>,
		axis_fixed<3>
	>;
	STATIC_CHECK( core::slot_count == 3 );
	STATIC_CHECK( core::run_count == 1 );
	STATIC_CHECK( core::axis_count == 2 );
	STATIC_CHECK(
		std::is_same<
			core::slot_list,
			type_list<
				axis_run<positive_extent_domain>,
				axis_same_as<0>,
				axis_fixed<3>
			>
		>::value
	);
}
