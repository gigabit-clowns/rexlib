// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/rules/axis_slot_descriptor.hpp>

#include <rexlib/core/dispatch/rules/any_extent_domain.hpp>
#include <rexlib/core/dispatch/rules/half_spectrum_extent_transform.hpp>
#include <rexlib/core/dispatch/rules/positive_extent_domain.hpp>

#include <string>

using namespace rexlib;

TEST_CASE(
	"axis_slot_descriptor should store how its axis gets its extent",
	"[axis_slot_descriptor]"
)
{
	const axis_slot_descriptor descriptor(
		axis_slot_kind::extent,
		2,
		&half_spectrum_extent_transform::apply,
		half_spectrum_extent_transform::get_relation(),
		make_extent_domain<any_extent_domain>()
	);
	CHECK( descriptor.get_kind() == axis_slot_kind::extent );
	CHECK( descriptor.get_index() == 2 );
	CHECK( descriptor.apply(16) == 9 );
	CHECK(
		std::string(descriptor.get_relation()) ==
		half_spectrum_extent_transform::get_relation()
	);
	CHECK( descriptor.get_domain().contains(0) );
}

TEST_CASE(
	"axis_slot_descriptor should store the domain of a run",
	"[axis_slot_descriptor]"
)
{
	const axis_slot_descriptor descriptor(
		axis_slot_kind::run,
		0,
		&half_spectrum_extent_transform::apply,
		half_spectrum_extent_transform::get_relation(),
		make_extent_domain<positive_extent_domain>()
	);
	CHECK( descriptor.get_kind() == axis_slot_kind::run );
	CHECK_FALSE( descriptor.get_domain().contains(0) );
}
