// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/image_region_clipping.hpp>

#include <rexlib/em/image/image_transfer_plan.hpp>

#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

const std::vector<std::size_t> image_extents = {100, 100};
const std::vector<std::size_t> patch_extents = {10, 10};

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

// One patch of a two dimensional image landing in one slot of a three
// dimensional batch, which is the shape the patch source produces.
void add_patch(
	image_transfer_plan &regions,
	std::size_t file_row,
	std::size_t file_column,
	std::size_t slot,
	std::size_t patch_row,
	std::size_t patch_column
)
{
	const std::size_t file_offset[2] = {file_row, file_column};
	const std::size_t array_offset[3] = {slot, patch_row, patch_column};
	regions.add(make_span(file_offset, 2), make_span(array_offset, 3));
}

image_transfer_plan make_patch_plan()
{
	return image_transfer_plan(make_span(patch_extents), 2, 3);
}

// The destination of a batch of `count` patches.
std::vector<std::size_t> make_array_extents(std::size_t count)
{
	return std::vector<std::size_t>{
		count,
		patch_extents[0],
		patch_extents[1]
	};
}

} // anonymous namespace

TEST_CASE(
	"make_clipped_transfer_plans checks the rank of each side",
	"[image_region_clipping]"
)
{
	const auto regions = make_patch_plan();
	const auto array_extents = make_array_extents(1);
	std::vector<image_transfer_plan> result;

	SECTION( "file extents that do not have the file rank" )
	{
		const std::vector<std::size_t> rank_three = {100, 100, 100};

		REQUIRE_THROWS_AS(
			make_clipped_transfer_plans(
				regions,
				make_span(rank_three),
				make_span(array_extents),
				result
			),
			std::invalid_argument
		);
	}

	SECTION( "array extents that do not have the array rank" )
	{
		const std::vector<std::size_t> rank_two = {100, 100};

		REQUIRE_THROWS_AS(
			make_clipped_transfer_plans(
				regions,
				make_span(image_extents),
				make_span(rank_two),
				result
			),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"make_clipped_transfer_plans leaves regions that fit alone",
	"[image_region_clipping]"
)
{
	auto regions = make_patch_plan();
	add_patch(regions, 0, 0, 0, 0, 0);
	add_patch(regions, 40, 50, 1, 0, 0);
	add_patch(regions, 90, 90, 2, 0, 0);

	const auto array_extents = make_array_extents(3);
	std::vector<image_transfer_plan> result;

	// The last patch ends exactly at the far edge of the image, which fits.
	REQUIRE_FALSE(
		make_clipped_transfer_plans(
			regions,
			make_span(image_extents),
			make_span(array_extents),
			result
		)
	);
	CHECK( result.empty() );
}

TEST_CASE(
	"make_clipped_transfer_plans shortens a region running off the file",
	"[image_region_clipping]"
)
{
	auto regions = make_patch_plan();
	add_patch(regions, 95, 93, 0, 0, 0);

	const auto array_extents = make_array_extents(1);
	std::vector<image_transfer_plan> result;

	REQUIRE(
		make_clipped_transfer_plans(
			regions,
			make_span(image_extents),
			make_span(array_extents),
			result
		)
	);

	REQUIRE( result.size() == 1 );
	CHECK( to_vector(result[0].get_extents()) ==
		std::vector<std::size_t>{5, 7} );
	REQUIRE( result[0].get_region_count() == 1 );
	CHECK( to_vector(result[0].get_file_offset(0)) ==
		std::vector<std::size_t>{95, 93} );
	CHECK( to_vector(result[0].get_array_offset(0)) ==
		std::vector<std::size_t>{0, 0, 0} );
}

TEST_CASE(
	"make_clipped_transfer_plans shortens a region the array offset pushes "
	"over the end of its slot",
	"[image_region_clipping]"
)
{
	// How a patch centred near the origin arrives: the part of it before the
	// image begins is carried by the array offset, and the region still
	// carries the extents of a whole patch.
	auto regions = make_patch_plan();
	add_patch(regions, 0, 0, 0, 4, 6);

	const auto array_extents = make_array_extents(1);
	std::vector<image_transfer_plan> result;

	REQUIRE(
		make_clipped_transfer_plans(
			regions,
			make_span(image_extents),
			make_span(array_extents),
			result
		)
	);

	REQUIRE( result.size() == 1 );
	CHECK( to_vector(result[0].get_extents()) ==
		std::vector<std::size_t>{6, 4} );
	REQUIRE( result[0].get_region_count() == 1 );
	CHECK( to_vector(result[0].get_file_offset(0)) ==
		std::vector<std::size_t>{0, 0} );
	CHECK( to_vector(result[0].get_array_offset(0)) ==
		std::vector<std::size_t>{0, 4, 6} );
}

TEST_CASE(
	"make_clipped_transfer_plans shortens by whichever side runs out first",
	"[image_region_clipping]"
)
{
	// A patch wider than the image it is cut from: it begins before the
	// image on one axis and ends after it on the other.
	const std::vector<std::size_t> small_image = {8, 8};

	auto regions = make_patch_plan();
	add_patch(regions, 0, 3, 0, 2, 0);

	const auto array_extents = make_array_extents(1);
	std::vector<image_transfer_plan> result;

	REQUIRE(
		make_clipped_transfer_plans(
			regions,
			make_span(small_image),
			make_span(array_extents),
			result
		)
	);

	REQUIRE( result.size() == 1 );
	CHECK( to_vector(result[0].get_extents()) ==
		std::vector<std::size_t>{8, 5} );
}

TEST_CASE(
	"make_clipped_transfer_plans drops a region that reaches nothing",
	"[image_region_clipping]"
)
{
	const auto array_extents = make_array_extents(2);
	std::vector<image_transfer_plan> result;

	SECTION( "one starting past the end of the file" )
	{
		auto regions = make_patch_plan();
		add_patch(regions, 100, 0, 0, 0, 0);
		add_patch(regions, 40, 40, 1, 0, 0);

		REQUIRE(
			make_clipped_transfer_plans(
				regions,
				make_span(image_extents),
				make_span(array_extents),
				result
			)
		);

		REQUIRE( result.size() == 1 );
		CHECK( to_vector(result[0].get_extents()) == patch_extents );
		REQUIRE( result[0].get_region_count() == 1 );
		CHECK( to_vector(result[0].get_array_offset(0)) ==
			std::vector<std::size_t>{1, 0, 0} );
	}

	SECTION( "one starting past the end of its slot" )
	{
		auto regions = make_patch_plan();
		add_patch(regions, 0, 0, 0, 10, 0);
		add_patch(regions, 40, 40, 1, 0, 0);

		REQUIRE(
			make_clipped_transfer_plans(
				regions,
				make_span(image_extents),
				make_span(array_extents),
				result
			)
		);

		REQUIRE( result.size() == 1 );
		REQUIRE( result[0].get_region_count() == 1 );
		CHECK( to_vector(result[0].get_array_offset(0)) ==
			std::vector<std::size_t>{1, 0, 0} );
	}

	SECTION( "one whose slot is not there at all" )
	{
		// The batch axis is a leading axis, which spans a single position
		// and so can never be shortened.
		auto regions = make_patch_plan();
		add_patch(regions, 40, 40, 2, 0, 0);
		add_patch(regions, 40, 40, 1, 0, 0);

		REQUIRE(
			make_clipped_transfer_plans(
				regions,
				make_span(image_extents),
				make_span(array_extents),
				result
			)
		);

		REQUIRE( result.size() == 1 );
		REQUIRE( result[0].get_region_count() == 1 );
		CHECK( to_vector(result[0].get_array_offset(0)) ==
			std::vector<std::size_t>{1, 0, 0} );
	}
}

TEST_CASE(
	"make_clipped_transfer_plans makes one plan per shape",
	"[image_region_clipping]"
)
{
	auto regions = make_patch_plan();
	add_patch(regions, 40, 40, 0, 0, 0);   // whole
	add_patch(regions, 95, 40, 1, 0, 0);   // 5 by 10
	add_patch(regions, 50, 50, 2, 0, 0);   // whole
	add_patch(regions, 40, 97, 3, 0, 0);   // 10 by 3
	add_patch(regions, 96, 40, 4, 0, 0);   // 4 by 10

	const auto array_extents = make_array_extents(5);
	std::vector<image_transfer_plan> result;

	REQUIRE(
		make_clipped_transfer_plans(
			regions,
			make_span(image_extents),
			make_span(array_extents),
			result
		)
	);

	// One per distinct shape, in the order the shapes were first met.
	REQUIRE( result.size() == 4 );

	CHECK( to_vector(result[0].get_extents()) == patch_extents );
	REQUIRE( result[0].get_region_count() == 2 );
	CHECK( to_vector(result[0].get_array_offset(0)) ==
		std::vector<std::size_t>{0, 0, 0} );
	CHECK( to_vector(result[0].get_array_offset(1)) ==
		std::vector<std::size_t>{2, 0, 0} );

	CHECK( to_vector(result[1].get_extents()) ==
		std::vector<std::size_t>{5, 10} );
	REQUIRE( result[1].get_region_count() == 1 );

	CHECK( to_vector(result[2].get_extents()) ==
		std::vector<std::size_t>{10, 3} );
	REQUIRE( result[2].get_region_count() == 1 );

	CHECK( to_vector(result[3].get_extents()) ==
		std::vector<std::size_t>{4, 10} );
	REQUIRE( result[3].get_region_count() == 1 );
}

TEST_CASE(
	"make_clipped_transfer_plans keeps the ranks of the plan it clips",
	"[image_region_clipping]"
)
{
	// A patch of one slice of a stack: the file carries an axis the extents
	// do not reach, as the array does.
	const std::vector<std::size_t> stack_extents = {6, 100, 100};
	image_transfer_plan regions(make_span(patch_extents), 3, 3);

	const std::size_t file_offset[3] = {4, 95, 40};
	const std::size_t array_offset[3] = {0, 0, 0};
	regions.add(make_span(file_offset, 3), make_span(array_offset, 3));

	const auto array_extents = make_array_extents(1);
	std::vector<image_transfer_plan> result;

	REQUIRE(
		make_clipped_transfer_plans(
			regions,
			make_span(stack_extents),
			make_span(array_extents),
			result
		)
	);

	REQUIRE( result.size() == 1 );
	CHECK( result[0].get_file_rank() == 3 );
	CHECK( result[0].get_array_rank() == 3 );
	CHECK( result[0].get_rank() == 2 );
	CHECK( to_vector(result[0].get_extents()) ==
		std::vector<std::size_t>{5, 10} );
	CHECK( to_vector(result[0].get_file_offset(0)) ==
		std::vector<std::size_t>{4, 95, 40} );
}

TEST_CASE(
	"make_clipped_transfer_plans clears what it is given",
	"[image_region_clipping]"
)
{
	auto regions = make_patch_plan();
	add_patch(regions, 95, 40, 0, 0, 0);

	const auto array_extents = make_array_extents(1);
	std::vector<image_transfer_plan> result;
	result.emplace_back(make_span(patch_extents), 2, 3);

	REQUIRE(
		make_clipped_transfer_plans(
			regions,
			make_span(image_extents),
			make_span(array_extents),
			result
		)
	);

	REQUIRE( result.size() == 1 );
	CHECK( to_vector(result[0].get_extents()) ==
		std::vector<std::size_t>{5, 10} );
}
