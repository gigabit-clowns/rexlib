// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/core/operation/shape_rule/positive_extent_domain.hpp>

#include <cstddef>
#include <string>

using namespace rexlib;

TEST_CASE(
	"positive_extent_domain should admit every extent but zero",
	"[positive_extent_domain]"
)
{
	const auto extent = GENERATE(std::size_t(1), std::size_t(7));
	CHECK( positive_extent_domain::contains(extent) );
	CHECK_FALSE( positive_extent_domain::contains(0) );
}

TEST_CASE(
	"positive_extent_domain should describe what it admits",
	"[positive_extent_domain]"
)
{
	CHECK(
		std::string(positive_extent_domain::get_description()) ==
		"at least one"
	);
}
