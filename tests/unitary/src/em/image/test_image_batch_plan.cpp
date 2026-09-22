// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <em/image/image_batch_plan.hpp>

#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

const char *const caller = "a_caller::a_method";

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
	return make_batch_plan(
		make_span(array_extents),
		make_span(locations),
		caller
	);
}

} // namespace

TEST_CASE(
	"make_batch_plan checks the array against the locations",
	"[image_batch_plan]"
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
	"[image_batch_plan]"
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
	"make_batch_plan names its caller in what it throws",
	"[image_batch_plan]"
)
{
	// The caller is what a reader of the message knows they called; this is
	// shared, and naming itself would tell them nothing.
	const std::vector<image_location> locations;

	REQUIRE_THROWS_MATCHES(
		plan_of({}, locations),
		std::invalid_argument,
		Catch::Matchers::MessageMatches(
			Catch::Matchers::StartsWith(std::string(caller) + ": ")
		)
	);
}

TEST_CASE(
	"make_batch_plan holds an empty batch",
	"[image_batch_plan]"
)
{
	const std::vector<image_location> locations;
	const auto plan = plan_of({0, 4, 4}, locations);

	CHECK( plan.get_region_count() == 0 );
	CHECK( plan.get_file_count() == 0 );
}

TEST_CASE(
	"make_batch_plan addresses whole files at their origin",
	"[image_batch_plan]"
)
{
	// No position: a location names a file read or written as one image, so
	// the file rank is that of one element and every file offset is zero.
	const std::vector<image_location> locations = {
		image_location("a.mrc"),
		image_location("b.mrc"),
		image_location("a.mrc")
	};

	const auto plan = plan_of(batch_extents, locations);

	CHECK( plan.get_file_rank() == 2 );
	CHECK( plan.get_array_rank() == 3 );
	CHECK( to_vector(plan.get_extents()) == element_extents );
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
	"make_batch_plan takes a position as the leading file offset",
	"[image_batch_plan]"
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

	CHECK( plan.get_file_rank() == 3 );
	CHECK( plan.get_array_rank() == 3 );
	CHECK( to_vector(plan.get_extents()) == element_extents );
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
	"[image_batch_plan]"
)
{
	// Even consecutive positions of one file, which are one hyperrectangle
	// and could be described as one: saying so needs a set of extents of its
	// own, and a plan holds one for every region in it.
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 6),
		image_location("stack.mrcs", 7),
		image_location("stack.mrcs", 8)
	};

	const auto plan = plan_of(batch_extents, locations);

	CHECK( plan.get_region_count() == 3 );
	CHECK( to_vector(plan.get_extents()) == element_extents );
	CHECK( to_vector(plan.get_file_offset(0)) ==
		std::vector<std::size_t>{6, 0, 0} );
	CHECK( to_vector(plan.get_file_offset(2)) ==
		std::vector<std::size_t>{8, 0, 0} );
}
TEST_CASE(
	"make_batch_plan keeps every slot apart however they are placed",
	"[image_batch_plan]"
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
		CHECK( to_vector(plan.get_extents()) == element_extents );
	}

	SECTION( "consecutive positions of different files" )
	{
		const std::vector<image_location> locations = {
			image_location("first.mrcs", 0),
			image_location("second.mrcs", 1),
			image_location("first.mrcs", 2)
		};

		const auto plan = plan_of(batch_extents, locations);

		CHECK( plan.get_region_count() == 3 );
		CHECK( to_vector(plan.get_extents()) == element_extents );
	}

	SECTION( "one file's positions in descending order" )
	{
		const std::vector<image_location> locations = {
			image_location("stack.mrcs", 2),
			image_location("stack.mrcs", 1),
			image_location("stack.mrcs", 0)
		};

		const auto plan = plan_of(batch_extents, locations);

		CHECK( plan.get_region_count() == 3 );
		CHECK( to_vector(plan.get_extents()) == element_extents );
	}

	SECTION( "a batch of a single slot" )
	{
		const std::vector<image_location> locations = {
			image_location("stack.mrcs", 4)
		};

		const auto plan = plan_of({1, 4, 4}, locations);

		REQUIRE( plan.get_region_count() == 1 );
		CHECK( to_vector(plan.get_extents()) == element_extents );
		CHECK( to_vector(plan.get_file_offset(0)) ==
			std::vector<std::size_t>{4, 0, 0} );
	}
}
