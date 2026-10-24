// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <functional/array_axes.hpp>

#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>

#include "../core/hardware/mock/mock_buffer.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace rexlib;

namespace
{

array make_array(std::vector<std::size_t> extents)
{
	return array(
		std::make_shared<mock_buffer>(),
		array_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			numerical_type::float32
		)
	);
}

} // anonymous namespace

TEST_CASE(
	"get_rank should count the axes of an array",
	"[array_axes]"
)
{
	CHECK( get_rank(make_array({ 4, 5, 6 })) == 3 );
	CHECK( get_rank(make_array({})) == 0 );
}

TEST_CASE(
	"resolve_axes should count negative axes from the end",
	"[array_axes]"
)
{
	const auto x = make_array({ 4, 5, 6 });
	const std::array<std::ptrdiff_t, 3> axes = {{ -1, 0, -3 }};

	CHECK( resolve_axes(x, make_span(axes)) == ops::axis_list{ 2, 0, 0 } );
}

TEST_CASE(
	"resolve_axes should reject an axis beyond the rank",
	"[array_axes]"
)
{
	const auto x = make_array({ 4, 5 });
	const std::array<std::ptrdiff_t, 1> beyond = {{ 2 }};
	const std::array<std::ptrdiff_t, 1> before = {{ -3 }};

	CHECK_THROWS_AS(resolve_axes(x, make_span(beyond)), std::out_of_range);
	CHECK_THROWS_AS(resolve_axes(x, make_span(before)), std::out_of_range);
}

TEST_CASE(
	"every_axis should name every axis in order",
	"[array_axes]"
)
{
	CHECK( every_axis(make_array({ 4, 5, 6 })) == ops::axis_list{ 0, 1, 2 } );
	CHECK( every_axis(make_array({})).empty() );
}
