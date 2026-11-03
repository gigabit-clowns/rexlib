// SPDX-License-Identifier: GPL-3.0-only

#include <core/layout/axis_values.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <vector>

using namespace rexlib;

TEST_CASE(
	"select_axes should give the value of each axis in the order asked",
	"[select_axes]"
)
{
	const std::vector<std::ptrdiff_t> strides = { 60, 20, 5, 1 };
	const std::vector<std::size_t> axes = { 3, 0, 2 };

	const auto result = select_axes(make_span(strides), make_span(axes));

	CHECK( result == std::vector<std::ptrdiff_t>{ 1, 60, 5 } );
}

TEST_CASE(
	"select_axes should repeat an axis asked twice",
	"[select_axes]"
)
{
	const std::vector<std::size_t> extents = { 4, 7 };
	const std::vector<std::size_t> axes = { 1, 1 };

	const auto result = select_axes(make_span(extents), make_span(axes));

	CHECK( result == std::vector<std::size_t>{ 7, 7 } );
}

TEST_CASE(
	"select_axes should select from values that are not const",
	"[select_axes]"
)
{
	std::vector<std::size_t> extents = { 4, 7, 9 };
	const std::vector<std::size_t> axes = { 2 };

	const auto result = select_axes(make_span(extents), make_span(axes));
	extents[2] = 1;

	CHECK( result == std::vector<std::size_t>{ 9 } );
}

TEST_CASE(
	"select_axes should give nothing when no axis is asked",
	"[select_axes]"
)
{
	const std::vector<std::size_t> extents = { 4, 7 };

	const auto result = select_axes(
		make_span(extents),
		span<const std::size_t>()
	);

	CHECK( result.empty() );
}
