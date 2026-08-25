// SPDX-License-Identifier: GPL-3.0-only

#include <core/layout/strided_axis.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <utility>

using namespace rexlib;

TEST_CASE("make_contiguous_axis should create an axis of stride 1", "[strided_axis]")
{
	const auto axis = make_contiguous_axis(1234);
	REQUIRE( axis.get_extent() == 1234 );
	REQUIRE( axis.get_stride() == 1 );
}

TEST_CASE("make_phantom_axis should create an axis of stride 0", "[strided_axis]")
{
	const auto axis = make_phantom_axis(9876);
	REQUIRE( axis.get_extent() == 9876 );
	REQUIRE( axis.get_stride() == 0 );
}

TEST_CASE("is_contiguous should return true only if the axis has a stride of 1", "[strided_axis]")
{
	REQUIRE( is_contiguous(strided_axis(1234, 1)) );
	REQUIRE( !is_contiguous(strided_axis(1234, 0)) );
	REQUIRE( !is_contiguous(strided_axis(1234, 8)) );
	REQUIRE( !is_contiguous(strided_axis(1234, -1)) );
}

TEST_CASE("is_contiguous should return true only if the product condition satisfies", "[strided_axis]")
{
	REQUIRE( is_contiguous(strided_axis(8, 2), strided_axis(1234, 16)) );
	REQUIRE( !is_contiguous(strided_axis(8, -2), strided_axis(1234, 16)) );
	REQUIRE( !is_contiguous(strided_axis(8, 2), strided_axis(1234, -16)) );
	REQUIRE( !is_contiguous(strided_axis(9, 2), strided_axis(1234, 16)) );
	REQUIRE( !is_contiguous(strided_axis(8, 2), strided_axis(1234, 31)) );
}

TEST_CASE("is_reversed should only return true when stride is negative", "[strided_axis]")
{
	REQUIRE( is_reversed(strided_axis(1234, -1)) );
	REQUIRE( is_reversed(strided_axis(1234, -45)) );
	REQUIRE( !is_reversed(strided_axis(1234, 0)) );
	REQUIRE( !is_reversed(strided_axis(1234, 83)) );
}

TEST_CASE("is_repeating should only return true for axes with an extent larger than 1 and an stride of zero", "[strided_axis]")
{
	REQUIRE( is_repeating(strided_axis(2, 0)) );
	REQUIRE( is_repeating(strided_axis(1743, 0)) );
	REQUIRE( !is_repeating(strided_axis(1, 0)) );
	REQUIRE( !is_repeating(strided_axis(0, 0)) );
	REQUIRE( !is_repeating(strided_axis(1244, 8)) );
	REQUIRE( !is_repeating(strided_axis(1244, -8)) );
}

TEST_CASE("is_empty should only return true for axes with an extent of zero", "[strided_axis]")
{
	REQUIRE( is_empty(strided_axis(0, 13)) );
	REQUIRE( is_empty(strided_axis(0, 0)) );
	REQUIRE( !is_empty(strided_axis(1, 0)) );
	REQUIRE( !is_empty(strided_axis(14, 0)) );
	REQUIRE( !is_empty(strided_axis(14, -18)) );
}

TEST_CASE("is_significant should only return true for axes with an extent unequal to 1", "[strided_axis]")
{
	REQUIRE( is_significant(strided_axis(12, 0)) );
	REQUIRE( is_significant(strided_axis(0, 0)) );
	REQUIRE( is_significant(strided_axis(12, 12)) );
	REQUIRE( is_significant(strided_axis(12, -12)) );
	REQUIRE( is_significant(strided_axis(43, 1)) );
	REQUIRE( is_significant(strided_axis(54, -1)) );
	REQUIRE( !is_significant(strided_axis(1, 0)) );
	REQUIRE( !is_significant(strided_axis(1, 18)) );
	REQUIRE( !is_significant(strided_axis(1, -18)) );
}

TEST_CASE("get_axis_last_offset should return the last offset for non-empty axes", "[strided_axis]")
{
	std::ptrdiff_t offset;
	REQUIRE( get_axis_last_offset(strided_axis(8, 10), offset) );
	REQUIRE( offset == 70 );
	REQUIRE( get_axis_last_offset(strided_axis(6, -10), offset) );
	REQUIRE( offset == -50 );
}

TEST_CASE("get_axis_last_offset should fail for empty axes", "[strided_axis]")
{
	std::ptrdiff_t offset = 1234567890;
	REQUIRE( !get_axis_last_offset(strided_axis(0, 10), offset) );
	REQUIRE( offset == 1234567890 );
}

TEST_CASE("get_axis_pivot_offset should return 0 for positive strided axes", "[strided_axis]")
{
	REQUIRE( get_axis_pivot_offset(strided_axis(1, 0)) == 0 );
	REQUIRE( get_axis_pivot_offset(strided_axis(18, 1)) == 0 );
	REQUIRE( get_axis_pivot_offset(strided_axis(18, 2384)) == 0 );
}

TEST_CASE("get_axis_pivot_offset should return 0 for empty axes", "[strided_axis]")
{
	REQUIRE( get_axis_pivot_offset(strided_axis(0, 0)) == 0 );
	REQUIRE( get_axis_pivot_offset(strided_axis(0, 1)) == 0 );
	REQUIRE( get_axis_pivot_offset(strided_axis(0, -1)) == 0 );
}

TEST_CASE("get_axis_pivot_offset should return the last offset for reversed axes", "[strided_axis]")
{
	REQUIRE( get_axis_pivot_offset(strided_axis(18, -1)) == 17 );
	REQUIRE( get_axis_pivot_offset(strided_axis(11, -10)) == 100 );
}

TEST_CASE("broadcast with equal extents should succeed and leave axes unmodified", "[strided_axis]")
{
	const strided_axis initial_axis1(123, 345);
	const strided_axis initial_axis2(123, 654);

	auto axis1 = initial_axis1;
	auto axis2 = initial_axis2;
	REQUIRE( broadcast(axis1, axis2) );
	REQUIRE(axis1 == initial_axis1 );
	REQUIRE(axis2 == initial_axis2 );
}

TEST_CASE("broadcast with left axis of extent 1 should broadcast it")
{
	const strided_axis initial_axis1(1, 345);
	const strided_axis initial_axis2(123, 654);

	auto axis1 = initial_axis1;
	auto axis2 = initial_axis2;
	REQUIRE( broadcast(axis1, axis2) );
	REQUIRE(axis1 == make_phantom_axis(axis2.get_extent()) );
	REQUIRE(axis2 == initial_axis2 );
}

TEST_CASE("broadcast with right axis of extent 1 should broadcast it")
{
	const strided_axis initial_axis1(123, 345);
	const strided_axis initial_axis2(1, 654);

	auto axis1 = initial_axis1;
	auto axis2 = initial_axis2;
	REQUIRE( broadcast(axis1, axis2) );
	REQUIRE(axis1 == initial_axis1 );
	REQUIRE(axis2 == make_phantom_axis(axis1.get_extent()) );
}

TEST_CASE("broadcast with unequal extents should fail and leave axes unmodified", "[strided_axis]")
{
	std::size_t size1;
	std::size_t size2;
	std::tie(size1, size2) = GENERATE(
		table<std::size_t, std::size_t>({
			{76, 0},
			{0, 3},
			{2, 3},
			{100, 101}
		})
	);

	const strided_axis initial_axis1(size1, 345);
	const strided_axis initial_axis2(size2, 654);

	auto axis1 = initial_axis1;
	auto axis2 = initial_axis2;
	REQUIRE( !broadcast(axis1, axis2) );
	REQUIRE(axis1 == initial_axis1 );
	REQUIRE(axis2 == initial_axis2 );
}

TEST_CASE("broadcast_to with equal extents should succeed and leave axis unmodified", "[strided_axis]")
{
	const std::size_t extent = 876;
	const auto initial_axis = make_contiguous_axis(extent);

	auto axis = initial_axis;
	REQUIRE( broadcast_to(axis, extent) );
	REQUIRE( axis == initial_axis );
}

TEST_CASE("broadcast_to with an axis of extent 1 should broadcast it")
{
	const std::size_t extent = 565;

	auto axis = make_contiguous_axis();
	REQUIRE( broadcast_to(axis, extent) );
	REQUIRE( axis == make_phantom_axis(extent) );
}

TEST_CASE("broadcast_to with unequal extents should fail and leave axis unmodified", "[strided_axis]")
{
	std::size_t size1;
	std::size_t size2;
	std::tie(size1, size2) = GENERATE(
		table<std::size_t, std::size_t>({
			{76, 0},
			{0, 3},
			{2, 3},
			{100, 101}
		})
	);

	const auto initial_axis = make_contiguous_axis(size1);

	auto axis = initial_axis;
	REQUIRE( !broadcast_to(axis, size2) );
	REQUIRE( axis == initial_axis );
}

TEST_CASE("apply_index should increment the offset as expected", "[strided_axis]")
{
	const std::ptrdiff_t initial_offset = 24;
	const strided_axis axis(16, -5);
	const std::size_t index = 3;

	auto offset = initial_offset;
	apply_index(axis, offset, index);
	REQUIRE( offset == 9 );
}

TEST_CASE("apply_index should throw with an out of bounds index", "[strided_axis]")
{
	const std::ptrdiff_t initial_offset = 24;
	const strided_axis axis(16, -5);
	const std::size_t index = 16;

	auto offset = initial_offset;
	REQUIRE_THROWS_AS( apply_index(axis, offset, index), std::out_of_range );
}

TEST_CASE("apply_slice should increment the offset and modify the axis", "[strided_axis]")
{
	const std::ptrdiff_t initial_offset = 24;
	const strided_axis initial_axis(16, -5);
	const slice slice(2, 5, 3);

	auto axis = initial_axis;
	auto offset = initial_offset;
	apply_slice(axis, offset, slice);
	REQUIRE( offset == 14 );
	REQUIRE( axis.get_extent() == 5 );
	REQUIRE( axis.get_stride() == -15 );
}

TEST_CASE("apply_slice should throw with an out of range index", "[strided_axis]")
{
	const std::ptrdiff_t initial_offset = 24;
	const strided_axis initial_axis(16, -5);
	const slice slice(12, 5, 3);

	auto axis = initial_axis;
	auto offset = initial_offset;
	REQUIRE_THROWS_AS( apply_slice(axis, offset, slice), std::out_of_range );
}

TEST_CASE("hash of strided_axis should change if any of the components changes", "[strided_axis]")
{
	strided_axis axis(12, 15);
	CHECK( axis.hash() == 0xcdbe0b208405a297 );
	axis.set_extent(11);
	CHECK( axis.hash() == 0x60cd1d2ee0fd46ab );
	axis.set_stride(43);
	CHECK( axis.hash() == 0x5654c5717cd925d1 );
}
