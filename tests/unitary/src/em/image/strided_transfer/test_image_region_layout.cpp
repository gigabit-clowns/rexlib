// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/strided_transfer/image_region_layout.hpp>

#include <rexlib/em/image/image_transfer_plan.hpp>

#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

image_transfer_plan one_region(
	const std::vector<std::size_t> &extents,
	std::size_t file_rank,
	std::size_t array_rank
)
{
	image_transfer_plan regions(make_span(extents), file_rank, array_rank);
	regions.add(
		make_span(std::vector<std::size_t>(file_rank, 0)),
		make_span(std::vector<std::size_t>(array_rank, 0))
	);

	return regions;
}

} // anonymous namespace

TEST_CASE( "the destination decides the order the axes are walked in",
	"[image_region_layout]" )
{
	// Neither operand is contiguous in the other's order, so the two disagree
	// about which axis should be innermost and nothing coalesces. That is
	// what makes the choice observable.
	const std::vector<std::size_t> extents = {4, 8};
	const std::vector<std::ptrdiff_t> row_major = {8, 1};
	const std::vector<std::ptrdiff_t> column_major = {1, 4};

	const auto regions = one_region(extents, 2, 2);

	SECTION( "the first operand is the one walked with a unit stride" )
	{
		const auto rows_first = build_region_layout(
			regions, make_span(row_major), make_span(column_major));
		const auto columns_first = build_region_layout(
			regions, make_span(column_major), make_span(row_major));

		REQUIRE( rows_first.get_strides(0)[0] == 1 );
		REQUIRE( rows_first.get_strides(1)[0] != 1 );

		REQUIRE( columns_first.get_strides(0)[0] == 1 );
		REQUIRE( columns_first.get_strides(1)[0] != 1 );
	}

	SECTION( "both orders visit the same number of elements" )
	{
		const auto rows_first = build_region_layout(
			regions, make_span(row_major), make_span(column_major));
		const auto columns_first = build_region_layout(
			regions, make_span(column_major), make_span(row_major));

		REQUIRE( rows_first.compute_element_count() == 32 );
		REQUIRE( columns_first.compute_element_count() == 32 );
	}
}

TEST_CASE( "a side deeper than the region contributes only its inner axes",
	"[image_region_layout]" )
{
	// The extents of a batch cover the trailing axes of a side. The leading
	// ones are where a region starts, which is a pointer offset rather than
	// something the space is walked along, so their strides are not operands.
	const std::vector<std::size_t> extents = {3, 4};
	const std::vector<std::ptrdiff_t> file_strides = {12, 4, 1};
	const std::vector<std::ptrdiff_t> array_strides = {4, 1};

	const auto regions = one_region(extents, 3, 2);
	const auto layout = build_region_layout(
		regions, make_span(array_strides), make_span(file_strides));

	SECTION( "the space has the rank of the region, not of the file" )
	{
		// Both sides are contiguous over the region here, so the two axes
		// coalesce into one of twelve elements.
		REQUIRE( layout.compute_element_count() == 12 );
		REQUIRE( layout.get_rank() <= 2 );
	}

	SECTION( "both operands are walked contiguously" )
	{
		REQUIRE( layout.get_strides(0)[0] == 1 );
		REQUIRE( layout.get_strides(1)[0] == 1 );
	}
}

TEST_CASE( "a region of no elements yields a space that cannot be walked",
	"[image_region_layout]" )
{
	const std::vector<std::size_t> extents = {0, 4};
	const std::vector<std::ptrdiff_t> strides = {4, 1};

	const auto regions = one_region(extents, 2, 2);
	const auto layout = build_region_layout(
		regions, make_span(strides), make_span(strides));

	REQUIRE( layout.compute_element_count() == 0 );
}
