// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/shape_rule/extent_slot_descriptor.hpp>

#include "fixtures/doubled_extent_transform.hpp"

#include <string>

using namespace rexlib;

TEST_CASE(
	"extent_slot_descriptor should store how its axis gets its extent",
	"[extent_slot_descriptor]"
)
{
	const extent_slot_descriptor descriptor(
		2,
		make_extent_transform<test::doubled_extent_transform>()
	);

	CHECK( descriptor.get_pivot_index() == 2 );
	CHECK( descriptor.get_transform().apply(16) == 32 );
	CHECK(
		std::string(descriptor.get_transform().get_relation()) ==
		test::doubled_extent_transform::get_relation()
	);
	CHECK_FALSE( descriptor.get_transform().is_identity() );
}
