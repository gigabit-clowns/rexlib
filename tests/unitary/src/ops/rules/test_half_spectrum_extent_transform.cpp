// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/ops/rules/half_spectrum_extent_transform.hpp>

#include <cstddef>
#include <string>
#include <utility>

using namespace rexlib::ops;

TEST_CASE(
	"half_spectrum_extent_transform should give the coefficients a real "
	"signal keeps",
	"[half_spectrum_extent_transform]"
)
{
	const auto pair = GENERATE(
		std::make_pair(std::size_t(1), std::size_t(1)),
		std::make_pair(std::size_t(2), std::size_t(2)),
		std::make_pair(std::size_t(7), std::size_t(4)),
		std::make_pair(std::size_t(8), std::size_t(5))
	);
	CHECK( half_spectrum_extent_transform::apply(pair.first) == pair.second );
}

TEST_CASE(
	"half_spectrum_extent_transform should name its relation",
	"[half_spectrum_extent_transform]"
)
{
	CHECK(
		std::string(half_spectrum_extent_transform::get_relation()) ==
		"the half spectrum extent of"
	);
}
