// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/shape_rule/axis_slot_descriptor.hpp>

#include <rexlib/core/operation/shape_rule/any_extent_domain.hpp>
#include <rexlib/core/operation/shape_rule/positive_extent_domain.hpp>

#include "fixtures/doubled_extent_transform.hpp"

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
		&test::doubled_extent_transform::apply,
		test::doubled_extent_transform::get_relation(),
		false,
		make_extent_domain<any_extent_domain>()
	);
	CHECK( descriptor.get_kind() == axis_slot_kind::extent );
	CHECK( descriptor.get_index() == 2 );
	CHECK( descriptor.apply(16) == 32 );
	CHECK(
		std::string(descriptor.get_relation()) ==
		test::doubled_extent_transform::get_relation()
	);
	CHECK_FALSE( descriptor.determines_extent() );
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
		&test::doubled_extent_transform::apply,
		test::doubled_extent_transform::get_relation(),
		false,
		make_extent_domain<positive_extent_domain>()
	);
	CHECK( descriptor.get_kind() == axis_slot_kind::run );
	CHECK_FALSE( descriptor.get_domain().contains(0) );
}
