// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/loops/multidimensional_index.hpp>

#include <array>
#include <cstddef>

using namespace rexlib::cpu;

TEST_CASE(
	"multidimensional_index should read the coordinates of the element it is "
	"placed at",
	"[multidimensional_index]"
)
{
	const std::array<std::ptrdiff_t, 3> coordinates = {{ 4, 5, 6 }};
	const auto index =
		multidimensional_index(3, 1).rebased(coordinates.data());

	CHECK( index.get_rank() == 3 );
	CHECK( index.get_inner_axis() == 1 );
	CHECK( index[0] == 4 );
	CHECK( index[1] == 5 );
	CHECK( index[2] == 6 );
}

TEST_CASE(
	"multidimensional_index should advance along its inner axis alone",
	"[multidimensional_index]"
)
{
	const std::array<std::ptrdiff_t, 2> coordinates = {{ 4, 5 }};
	const auto index = multidimensional_index(2, 0)
		.rebased(coordinates.data())
		.advanced(3);

	CHECK( index[0] == 7 );
	CHECK( index[1] == 5 );
	CHECK( index.get_inner_axis() == 0 );
}

TEST_CASE(
	"multidimensional_index should name the rank as the inner axis of a "
	"traversal without axes",
	"[multidimensional_index]"
)
{
	const multidimensional_index index(0, 0);

	CHECK( index.get_rank() == 0 );
	CHECK( index.get_inner_axis() == 0 );
}
