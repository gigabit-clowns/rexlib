// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <em/image/image_plan_builders.hpp>

#include <rexlib/core/layout/index_table.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// A batch of three slots, each one image of four by four.
const std::vector<std::size_t> batch_extents = {3, 4, 4};
const std::vector<std::size_t> element_extents = {4, 4};

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

image_transaction_plan plan_of(
	const std::vector<std::size_t> &array_extents,
	const std::vector<image_location> &locations
)
{
	return make_batch_plan(make_span(array_extents), make_span(locations));
}

index_table make_centres(
	const std::vector<std::vector<std::size_t>> &centres,
	std::size_t rank
)
{
	index_table result(rank);
	for (const auto &centre : centres)
	{
		result.add(make_span(centre));
	}

	return result;
}

image_transaction_plan patch_plan_of(
	const std::vector<std::size_t> &array_extents,
	const image_location &location,
	const index_table &centres
)
{
	return make_patch_plan(make_span(array_extents), location, centres);
}

image_descriptor make_descriptor(
	const std::vector<std::size_t> &extents,
	std::size_t core_rank
)
{
	return image_descriptor(
		make_span(extents),
		core_rank,
		numerical_type::float32
	);
}

} // anonymous namespace

TEST_CASE(
	"make_batch_plan checks the array against the locations",
	"[image_plan_builders]"
)
{
	const std::vector<image_location> locations = {
		image_location("a.mrc"),
		image_location("b.mrc")
	};

	SECTION( "an array with no extents" )
	{
		REQUIRE_THROWS_AS(
			plan_of({}, locations),
			std::invalid_argument
		);
	}

	SECTION( "a leading extent that is not the number of locations" )
	{
		REQUIRE_THROWS_AS(
			plan_of(batch_extents, locations),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"make_batch_plan refuses a batch of both kinds of location",
	"[image_plan_builders]"
)
{
	// The two do not agree on the rank of the file, so a plan cannot hold
	// them together.
	SECTION( "an unindexed location following an indexed one" )
	{
		const std::vector<image_location> locations = {
			image_location("stack.mrcs", 0),
			image_location("plain.mrc")
		};

		REQUIRE_THROWS_AS(
			plan_of({2, 4, 4}, locations),
			std::invalid_argument
		);
	}

	SECTION( "an indexed location following an unindexed one" )
	{
		const std::vector<image_location> locations = {
			image_location("plain.mrc"),
			image_location("stack.mrcs", 0)
		};

		REQUIRE_THROWS_AS(
			plan_of({2, 4, 4}, locations),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"make_batch_plan names itself in what it throws",
	"[image_plan_builders]"
)
{
	const std::vector<image_location> locations;

	REQUIRE_THROWS_MATCHES(
		plan_of({}, locations),
		std::invalid_argument,
		Catch::Matchers::MessageMatches(
			Catch::Matchers::StartsWith("make_batch_plan: ")
		)
	);
}

TEST_CASE(
	"make_batch_plan holds an empty batch",
	"[image_plan_builders]"
)
{
	const std::vector<image_location> locations;
	const auto plan = plan_of({0, 4, 4}, locations);

	CHECK( plan.get_region_count() == 0 );
	CHECK( plan.get_file_count() == 0 );
}

TEST_CASE(
	"make_batch_plan addresses whole files at their origin",
	"[image_plan_builders]"
)
{
	// No index in a stack: a location names a file read or written as one
	// image, so the file rank is that of one element and every file offset is
	// zero.
	const std::vector<image_location> locations = {
		image_location("a.mrc"),
		image_location("b.mrc"),
		image_location("a.mrc")
	};

	const auto plan = plan_of(batch_extents, locations);

	CHECK( plan.get_shape().get_file_rank() == 2 );
	CHECK( plan.get_shape().get_array_rank() == 3 );
	CHECK( to_vector(plan.get_shape().get_extents()) == element_extents );
	REQUIRE( plan.get_region_count() == 3 );

	for (std::size_t i = 0; i < 3; ++i)
	{
		CHECK( to_vector(plan.get_file_offset(i)) ==
			std::vector<std::size_t>{0, 0} );
		CHECK( to_vector(plan.get_array_offset(i)) ==
			std::vector<std::size_t>{i, 0, 0} );
	}

	// The repeated path is one file of the plan, not two.
	CHECK( plan.get_file_count() == 2 );
	CHECK( plan.get_region_file(0) == plan.get_region_file(2) );
}

TEST_CASE(
	"make_batch_plan takes the index in the stack as the leading file offset",
	"[image_plan_builders]"
)
{
	// Every location carries one, so the file gains the axis it stacks
	// along and the array offset keeps tracking the slot.
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 2),
		image_location("stack.mrcs", 0),
		image_location("stack.mrcs", 5)
	};

	const auto plan = plan_of(batch_extents, locations);

	CHECK( plan.get_shape().get_file_rank() == 3 );
	CHECK( plan.get_shape().get_array_rank() == 3 );
	CHECK( to_vector(plan.get_shape().get_extents()) == element_extents );
	REQUIRE( plan.get_region_count() == 3 );
	CHECK( to_vector(plan.get_file_offset(0)) ==
		std::vector<std::size_t>{2, 0, 0} );
	CHECK( to_vector(plan.get_array_offset(0)) ==
		std::vector<std::size_t>{0, 0, 0} );
	CHECK( to_vector(plan.get_file_offset(1)) ==
		std::vector<std::size_t>{0, 0, 0} );
	CHECK( to_vector(plan.get_array_offset(1)) ==
		std::vector<std::size_t>{1, 0, 0} );
	CHECK( to_vector(plan.get_file_offset(2)) ==
		std::vector<std::size_t>{5, 0, 0} );
	CHECK( to_vector(plan.get_array_offset(2)) ==
		std::vector<std::size_t>{2, 0, 0} );
}

TEST_CASE(
	"make_batch_plan gives every slot a region of its own",
	"[image_plan_builders]"
)
{
	// Even consecutive indices in one stack, which are one hyperrectangle
	// and could be described as one: saying so needs a set of extents of its
	// own, and a plan holds one for every region in it.
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 6),
		image_location("stack.mrcs", 7),
		image_location("stack.mrcs", 8)
	};

	const auto plan = plan_of(batch_extents, locations);

	CHECK( plan.get_region_count() == 3 );
	CHECK( to_vector(plan.get_shape().get_extents()) == element_extents );
	CHECK( to_vector(plan.get_file_offset(0)) ==
		std::vector<std::size_t>{6, 0, 0} );
	CHECK( to_vector(plan.get_file_offset(2)) ==
		std::vector<std::size_t>{8, 0, 0} );
}
TEST_CASE(
	"make_batch_plan keeps every slot apart however they are placed",
	"[image_plan_builders]"
)
{
	// A plan carries one set of extents for every region it holds, whatever
	// the locations look like.
	SECTION( "a run that stops before the end of the batch" )
	{
		const std::vector<image_location> locations = {
			image_location("stack.mrcs", 0),
			image_location("stack.mrcs", 1),
			image_location("stack.mrcs", 4)
		};

		const auto plan = plan_of(batch_extents, locations);

		CHECK( plan.get_region_count() == 3 );
		CHECK( to_vector(plan.get_shape().get_extents()) == element_extents );
	}

	SECTION( "consecutive indices in different stacks" )
	{
		const std::vector<image_location> locations = {
			image_location("first.mrcs", 0),
			image_location("second.mrcs", 1),
			image_location("first.mrcs", 2)
		};

		const auto plan = plan_of(batch_extents, locations);

		CHECK( plan.get_region_count() == 3 );
		CHECK( to_vector(plan.get_shape().get_extents()) == element_extents );
	}

	SECTION( "one stack's indices in descending order" )
	{
		const std::vector<image_location> locations = {
			image_location("stack.mrcs", 2),
			image_location("stack.mrcs", 1),
			image_location("stack.mrcs", 0)
		};

		const auto plan = plan_of(batch_extents, locations);

		CHECK( plan.get_region_count() == 3 );
		CHECK( to_vector(plan.get_shape().get_extents()) == element_extents );
	}

	SECTION( "a batch of a single slot" )
	{
		const std::vector<image_location> locations = {
			image_location("stack.mrcs", 4)
		};

		const auto plan = plan_of({1, 4, 4}, locations);

		REQUIRE( plan.get_region_count() == 1 );
		CHECK( to_vector(plan.get_shape().get_extents()) == element_extents );
		CHECK( to_vector(plan.get_file_offset(0)) ==
			std::vector<std::size_t>{4, 0, 0} );
	}
}

TEST_CASE(
	"make_patch_plan checks the array against the centres",
	"[image_plan_builders]"
)
{
	const image_location location("a.mrc");

	SECTION( "an array with no extents" )
	{
		REQUIRE_THROWS_AS(
			patch_plan_of({}, location, make_centres({}, 2)),
			std::invalid_argument
		);
	}

	SECTION( "a leading extent that is not the number of centres" )
	{
		const auto centres = make_centres({{10, 10}, {20, 20}}, 2);

		REQUIRE_THROWS_AS(
			patch_plan_of({3, 10, 10}, location, centres),
			std::invalid_argument
		);
	}

	SECTION( "centres that do not have the rank of one patch" )
	{
		const auto centres = make_centres({{10, 10, 10}}, 3);

		REQUIRE_THROWS_AS(
			patch_plan_of({1, 10, 10}, location, centres),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"make_patch_plan names itself in what it throws",
	"[image_plan_builders]"
)
{
	REQUIRE_THROWS_MATCHES(
		patch_plan_of({}, image_location("a.mrc"), make_centres({}, 2)),
		std::invalid_argument,
		Catch::Matchers::MessageMatches(
			Catch::Matchers::StartsWith("make_patch_plan: ")
		)
	);
}

TEST_CASE(
	"make_patch_plan holds an empty batch",
	"[image_plan_builders]"
)
{
	const auto plan = patch_plan_of(
		{0, 10, 10},
		image_location("a.mrc"),
		make_centres({}, 2)
	);

	CHECK( plan.get_region_count() == 0 );
	CHECK( to_vector(plan.get_shape().get_extents()) ==
		std::vector<std::size_t>{10, 10} );
}

TEST_CASE(
	"make_patch_plan places each patch around its centre",
	"[image_plan_builders]"
)
{
	// The corner of a patch is its centre less half its extent, so a patch
	// of ten centred at fifty starts at forty-five, and one of nine centred
	// there starts at forty-six.
	const image_location location("a.mrc");
	const auto centres = make_centres({{50, 50}}, 2);

	SECTION( "an even extent" )
	{
		const auto plan = patch_plan_of({1, 10, 10}, location, centres);

		REQUIRE( plan.get_region_count() == 1 );
		CHECK( to_vector(plan.get_shape().get_extents()) ==
			std::vector<std::size_t>{10, 10} );
		CHECK( to_vector(plan.get_file_offset(0)) ==
			std::vector<std::size_t>{45, 45} );
	}

	SECTION( "an odd extent" )
	{
		const auto plan = patch_plan_of({1, 9, 9}, location, centres);

		REQUIRE( plan.get_region_count() == 1 );
		CHECK( to_vector(plan.get_shape().get_extents()) ==
			std::vector<std::size_t>{9, 9} );
		CHECK( to_vector(plan.get_file_offset(0)) ==
			std::vector<std::size_t>{46, 46} );
	}
}

TEST_CASE(
	"make_patch_plan gives each patch its own slot, all of one file",
	"[image_plan_builders]"
)
{
	const auto plan = patch_plan_of(
		{3, 10, 10},
		image_location("a.mrc"),
		make_centres({{20, 30}, {40, 50}, {60, 70}}, 2)
	);

	REQUIRE( plan.get_file_count() == 1 );
	CHECK( plan.get_file(0) == "a.mrc" );
	REQUIRE( plan.get_region_count() == 3 );
	CHECK( plan.get_shape().get_file_rank() == 2 );
	CHECK( plan.get_shape().get_array_rank() == 3 );

	CHECK( to_vector(plan.get_file_offset(0)) ==
		std::vector<std::size_t>{15, 25} );
	CHECK( to_vector(plan.get_array_offset(0)) ==
		std::vector<std::size_t>{0, 0, 0} );
	CHECK( to_vector(plan.get_file_offset(1)) ==
		std::vector<std::size_t>{35, 45} );
	CHECK( to_vector(plan.get_array_offset(1)) ==
		std::vector<std::size_t>{1, 0, 0} );
	CHECK( to_vector(plan.get_file_offset(2)) ==
		std::vector<std::size_t>{55, 65} );
	CHECK( to_vector(plan.get_array_offset(2)) ==
		std::vector<std::size_t>{2, 0, 0} );
}

TEST_CASE(
	"make_patch_plan carries the part of a patch before the image in its "
	"array offset",
	"[image_plan_builders]"
)
{
	// A patch of ten centred at three starts two rows before the image
	// begins. The region still names a whole patch, starting two rows into
	// its slot and at the first row of the image.
	const auto plan = patch_plan_of(
		{1, 10, 10},
		image_location("a.mrc"),
		make_centres({{3, 50}}, 2)
	);

	REQUIRE( plan.get_region_count() == 1 );
	CHECK( to_vector(plan.get_shape().get_extents()) ==
		std::vector<std::size_t>{10, 10} );
	CHECK( to_vector(plan.get_file_offset(0)) ==
		std::vector<std::size_t>{0, 45} );
	CHECK( to_vector(plan.get_array_offset(0)) ==
		std::vector<std::size_t>{0, 2, 0} );
}

TEST_CASE(
	"make_patch_plan cuts the patches of a stack out of one slice",
	"[image_plan_builders]"
)
{
	// A location carrying an index in a stack grows the file rank by the
	// axis the stack is indexed along, which every patch shares.
	const auto plan = patch_plan_of(
		{2, 10, 10},
		image_location("stack.mrcs", 4),
		make_centres({{20, 30}, {40, 50}}, 2)
	);

	REQUIRE( plan.get_region_count() == 2 );
	CHECK( plan.get_shape().get_file_rank() == 3 );
	CHECK( plan.get_shape().get_array_rank() == 3 );
	CHECK( to_vector(plan.get_file_offset(0)) ==
		std::vector<std::size_t>{4, 15, 25} );
	CHECK( to_vector(plan.get_array_offset(0)) ==
		std::vector<std::size_t>{0, 0, 0} );
	CHECK( to_vector(plan.get_file_offset(1)) ==
		std::vector<std::size_t>{4, 35, 45} );
	CHECK( to_vector(plan.get_array_offset(1)) ==
		std::vector<std::size_t>{1, 0, 0} );
}

TEST_CASE(
	"make_patch_plan cuts boxes out of a volume the same way",
	"[image_plan_builders]"
)
{
	const auto plan = patch_plan_of(
		{1, 8, 8, 8},
		image_location("tomogram.mrc"),
		make_centres({{20, 30, 40}}, 3)
	);

	REQUIRE( plan.get_region_count() == 1 );
	CHECK( plan.get_shape().get_file_rank() == 3 );
	CHECK( plan.get_shape().get_array_rank() == 4 );
	CHECK( to_vector(plan.get_shape().get_extents()) ==
		std::vector<std::size_t>{8, 8, 8} );
	CHECK( to_vector(plan.get_file_offset(0)) ==
		std::vector<std::size_t>{16, 26, 36} );
	CHECK( to_vector(plan.get_array_offset(0)) ==
		std::vector<std::size_t>{0, 0, 0, 0} );
}

TEST_CASE(
	"make_location_plan spans the whole file for a location with no index",
	"[image_plan_builders]"
)
{
	const std::vector<std::size_t> stack_extents = {6, 3, 4};
	const auto plan = make_location_plan(
		make_descriptor(stack_extents, 2),
		image_location("stack.mrcs")
	);

	CHECK( to_vector(plan.get_shape().get_extents()) == stack_extents );
	CHECK( plan.get_shape().get_file_rank() == 3 );
	CHECK( plan.get_shape().get_array_rank() == 3 );
	REQUIRE( plan.get_region_count() == 1 );
	CHECK( to_vector(plan.get_file_offset(0)) ==
		std::vector<std::size_t>{0, 0, 0} );
	CHECK( to_vector(plan.get_array_offset(0)) ==
		std::vector<std::size_t>{0, 0, 0} );
}

TEST_CASE(
	"make_location_plan spans one image or volume for a location with an "
	"index",
	"[image_plan_builders]"
)
{
	SECTION( "an image of a stack of images" )
	{
		const std::vector<std::size_t> stack_extents = {6, 3, 4};
		const auto plan = make_location_plan(
			make_descriptor(stack_extents, 2),
			image_location("stack.mrcs", 4)
		);

		CHECK( to_vector(plan.get_shape().get_extents()) ==
			std::vector<std::size_t>{3, 4} );
		CHECK( plan.get_shape().get_file_rank() == 3 );
		CHECK( plan.get_shape().get_array_rank() == 2 );
		REQUIRE( plan.get_region_count() == 1 );
		CHECK( to_vector(plan.get_file_offset(0)) ==
			std::vector<std::size_t>{4, 0, 0} );
		CHECK( to_vector(plan.get_array_offset(0)) ==
			std::vector<std::size_t>{0, 0} );
	}

	SECTION( "a volume of a stack of volumes" )
	{
		const std::vector<std::size_t> stack_extents = {5, 8, 8, 8};
		const auto plan = make_location_plan(
			make_descriptor(stack_extents, 3),
			image_location("volumes.mrc", 2)
		);

		CHECK( to_vector(plan.get_shape().get_extents()) ==
			std::vector<std::size_t>{8, 8, 8} );
		CHECK( plan.get_shape().get_file_rank() == 4 );
		CHECK( plan.get_shape().get_array_rank() == 3 );
		REQUIRE( plan.get_region_count() == 1 );
		CHECK( to_vector(plan.get_file_offset(0)) ==
			std::vector<std::size_t>{2, 0, 0, 0} );
	}
}
