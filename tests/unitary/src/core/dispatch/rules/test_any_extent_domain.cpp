// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/core/dispatch/rules/any_extent_domain.hpp>

#include <cstddef>
#include <string>

using namespace rexlib;

TEST_CASE(
	"any_extent_domain should admit every extent",
	"[any_extent_domain]"
)
{
	const auto extent = GENERATE(
		std::size_t(0),
		std::size_t(1),
		std::size_t(1024)
	);
	CHECK( any_extent_domain::contains(extent) );
}

TEST_CASE(
	"any_extent_domain should describe what it admits",
	"[any_extent_domain]"
)
{
	CHECK(
		std::string(any_extent_domain::get_description()) == "any extent"
	);
}
