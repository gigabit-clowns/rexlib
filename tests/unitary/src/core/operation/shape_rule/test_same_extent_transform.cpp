// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/core/operation/shape_rule/same_extent_transform.hpp>

#include <cstddef>
#include <string>

using namespace rexlib;

TEST_CASE(
	"same_extent_transform should leave the extent as it is",
	"[same_extent_transform]"
)
{
	const auto extent = GENERATE(
		std::size_t(0),
		std::size_t(1),
		std::size_t(17)
	);
	CHECK( same_extent_transform::apply(extent) == extent );
}

TEST_CASE(
	"same_extent_transform should name its relation",
	"[same_extent_transform]"
)
{
	CHECK(
		std::string(same_extent_transform::get_relation()) ==
		"the extent of"
	);
}
