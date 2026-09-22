// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/image_region_merging.hpp>

#include <rexlib/em/image/image_transfer_plan.hpp>

#include <cstddef>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// One element of a stack landing in one slot of a batch: the file and the
// array each carry an axis the extents do not reach, which is the axis a run
// spans.
const std::vector<std::size_t> element_extents = {3, 5};

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

void add_element(
	image_transfer_plan &regions,
	std::size_t position,
	std::size_t slot
)
{
	const std::size_t file_offset[3] = {position, 0, 0};
	const std::size_t array_offset[3] = {slot, 0, 0};
	regions.add(make_span(file_offset, 3), make_span(array_offset, 3));
}

image_transfer_plan make_element_plan()
{
	return image_transfer_plan(make_span(element_extents), 3, 3);
}

} // namespace

TEST_CASE(
	"make_merged_transfer_plans leaves regions that neighbour nothing",
	"[image_region_merging]"
)
{
	std::vector<image_transfer_plan> result;

	SECTION( "positions with a gap between them" )
	{
		auto regions = make_element_plan();
		add_element(regions, 0, 0);
		add_element(regions, 2, 1);
		add_element(regions, 4, 2);

		REQUIRE_FALSE( make_merged_transfer_plans(regions, result) );
		CHECK( result.empty() );
	}

	SECTION( "consecutive positions landing out of order" )
	{
		// Neighbours on the file side alone is not enough: the slots they
		// land in have to follow each other too.
		auto regions = make_element_plan();
		add_element(regions, 0, 2);
		add_element(regions, 1, 1);
		add_element(regions, 2, 0);

		REQUIRE_FALSE( make_merged_transfer_plans(regions, result) );
	}

	SECTION( "a side with no axis beyond the extents of the plan" )
	{
		// A batch of whole files: the file side spans a single position
		// everywhere its extents do not reach, so no two of them can follow
		// each other.
		image_transfer_plan regions(make_span(element_extents), 2, 3);
		const std::size_t file_offset[2] = {0, 0};
		const std::size_t first[3] = {0, 0, 0};
		const std::size_t second[3] = {1, 0, 0};
		regions.add(make_span(file_offset, 2), make_span(first, 3));
		regions.add(make_span(file_offset, 2), make_span(second, 3));

		REQUIRE_FALSE( make_merged_transfer_plans(regions, result) );
	}

	SECTION( "a single region, which neighbours nothing" )
	{
		auto regions = make_element_plan();
		add_element(regions, 4, 0);

		REQUIRE_FALSE( make_merged_transfer_plans(regions, result) );
	}
}

TEST_CASE(
	"make_merged_transfer_plans spans the axis a run runs along",
	"[image_region_merging]"
)
{
	// Three consecutive slices into three consecutive slots: one
	// hyperrectangle, carrying one extent more than the plan did.
	auto regions = make_element_plan();
	add_element(regions, 6, 0);
	add_element(regions, 7, 1);
	add_element(regions, 8, 2);

	std::vector<image_transfer_plan> result;
	REQUIRE( make_merged_transfer_plans(regions, result) );

	REQUIRE( result.size() == 1 );
	CHECK( result[0].get_file_rank() == 3 );
	CHECK( result[0].get_array_rank() == 3 );
	CHECK( to_vector(result[0].get_extents()) ==
		std::vector<std::size_t>{3, 3, 5} );
	REQUIRE( result[0].get_region_count() == 1 );
	CHECK( to_vector(result[0].get_file_offset(0)) ==
		std::vector<std::size_t>{6, 0, 0} );
	CHECK( to_vector(result[0].get_array_offset(0)) ==
		std::vector<std::size_t>{0, 0, 0} );
}

TEST_CASE(
	"make_merged_transfer_plans gives each length of run a plan",
	"[image_region_merging]"
)
{
	// What one transaction could never say: a plan holds one set of extents,
	// so runs of two and of three need one each, and the region left over
	// keeps the extents the plan came with.
	auto regions = make_element_plan();
	add_element(regions, 0, 0);
	add_element(regions, 1, 1);
	add_element(regions, 4, 2);
	add_element(regions, 5, 3);
	add_element(regions, 6, 4);
	add_element(regions, 9, 5);

	std::vector<image_transfer_plan> result;
	REQUIRE( make_merged_transfer_plans(regions, result) );

	// One per length, in the order their length was first met.
	REQUIRE( result.size() == 3 );

	CHECK( to_vector(result[0].get_extents()) ==
		std::vector<std::size_t>{2, 3, 5} );
	REQUIRE( result[0].get_region_count() == 1 );
	CHECK( to_vector(result[0].get_file_offset(0)) ==
		std::vector<std::size_t>{0, 0, 0} );

	CHECK( to_vector(result[1].get_extents()) ==
		std::vector<std::size_t>{3, 3, 5} );
	REQUIRE( result[1].get_region_count() == 1 );
	CHECK( to_vector(result[1].get_file_offset(0)) ==
		std::vector<std::size_t>{4, 0, 0} );
	CHECK( to_vector(result[1].get_array_offset(0)) ==
		std::vector<std::size_t>{2, 0, 0} );

	CHECK( to_vector(result[2].get_extents()) == element_extents );
	REQUIRE( result[2].get_region_count() == 1 );
	CHECK( to_vector(result[2].get_file_offset(0)) ==
		std::vector<std::size_t>{9, 0, 0} );
}

TEST_CASE(
	"make_merged_transfer_plans gathers the runs of one length together",
	"[image_region_merging]"
)
{
	auto regions = make_element_plan();
	add_element(regions, 0, 0);
	add_element(regions, 1, 1);
	add_element(regions, 4, 2);
	add_element(regions, 5, 3);

	std::vector<image_transfer_plan> result;
	REQUIRE( make_merged_transfer_plans(regions, result) );

	REQUIRE( result.size() == 1 );
	CHECK( to_vector(result[0].get_extents()) ==
		std::vector<std::size_t>{2, 3, 5} );
	REQUIRE( result[0].get_region_count() == 2 );
	CHECK( to_vector(result[0].get_file_offset(1)) ==
		std::vector<std::size_t>{4, 0, 0} );
	CHECK( to_vector(result[0].get_array_offset(1)) ==
		std::vector<std::size_t>{2, 0, 0} );
}

TEST_CASE(
	"make_merged_transfer_plans keeps a region a run cannot reach",
	"[image_region_merging]"
)
{
	// Consecutive positions of one file that do not land in consecutive
	// slots break the run, each side having to follow for either to.
	auto regions = make_element_plan();
	add_element(regions, 0, 0);
	add_element(regions, 1, 1);
	add_element(regions, 2, 3);

	std::vector<image_transfer_plan> result;
	REQUIRE( make_merged_transfer_plans(regions, result) );

	REQUIRE( result.size() == 2 );
	CHECK( to_vector(result[0].get_extents()) ==
		std::vector<std::size_t>{2, 3, 5} );
	CHECK( to_vector(result[1].get_extents()) == element_extents );
	REQUIRE( result[1].get_region_count() == 1 );
	CHECK( to_vector(result[1].get_array_offset(0)) ==
		std::vector<std::size_t>{3, 0, 0} );
}

TEST_CASE(
	"make_merged_transfer_plans clears what it is given",
	"[image_region_merging]"
)
{
	auto regions = make_element_plan();
	add_element(regions, 0, 0);
	add_element(regions, 1, 1);

	std::vector<image_transfer_plan> result;
	result.emplace_back(make_span(element_extents), 3, 3);

	REQUIRE( make_merged_transfer_plans(regions, result) );
	REQUIRE( result.size() == 1 );
	CHECK( to_vector(result[0].get_extents()) ==
		std::vector<std::size_t>{2, 3, 5} );
}
