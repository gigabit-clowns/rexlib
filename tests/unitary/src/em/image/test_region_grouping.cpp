// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/region_grouping.hpp>

#include <rexlib/em/image/image_transaction_plan.hpp>

#include <algorithm>
#include <cstddef>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

const std::vector<std::size_t> plane_extents = {3, 5};

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

// Add one whole element of a stack: element `position` of file `file` lands
// in slot `slot` of a three dimensional array.
void add_element(
	image_transaction_plan &plan,
	std::size_t file,
	std::size_t position,
	std::size_t slot
)
{
	const std::size_t file_offset[3] = {position, 0, 0};
	const std::size_t array_offset[3] = {slot, 0, 0};
	plan.add(file, make_span(file_offset, 3), make_span(array_offset, 3));
}

} // namespace

TEST_CASE( "a region_grouping starts empty", "[region_grouping]" )
{
	const region_grouping grouping;

	REQUIRE( grouping.get_size() == 0 );
	REQUIRE( grouping.get_file_count() == 0 );
}

TEST_CASE( "a region_grouping gathers the regions of each file together",
	"[region_grouping]" )
{
	// A batch drawn at random from three stacks, in the order a sampler
	// happened to produce it. This is the case the type exists for.
	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	const auto zero = plan.add_file("stack_0.mrcs");
	const auto one = plan.add_file("stack_1.mrcs");
	const auto two = plan.add_file("stack_2.mrcs");

	add_element(plan, one, 7, 0);
	add_element(plan, zero, 3, 1);
	add_element(plan, two, 1, 2);
	add_element(plan, one, 2, 3);
	add_element(plan, zero, 9, 4);

	region_grouping grouping;
	grouping.build(plan);

	SECTION( "the plan itself is left in the order it was built in" )
	{
		REQUIRE( plan.get_size() == 5 );
		REQUIRE( plan.get_region_file(0) == one );
		REQUIRE( to_vector(plan.get_file_offset(0)) ==
			std::vector<std::size_t>{7, 0, 0} );
	}

	SECTION( "each file becomes one contiguous range" )
	{
		REQUIRE( grouping.get_size() == 5 );
		REQUIRE( grouping.get_file_count() == 3 );
		REQUIRE( grouping.get_region_count(zero) == 2 );
		REQUIRE( grouping.get_region_count(one) == 2 );
		REQUIRE( grouping.get_region_count(two) == 1 );
		REQUIRE( grouping.get_first_position(zero) == 0 );
		REQUIRE( grouping.get_first_position(one) == 2 );
		REQUIRE( grouping.get_first_position(two) == 4 );

		for (std::size_t file = 0; file < grouping.get_file_count(); ++file)
		{
			const auto first = grouping.get_first_position(file);
			const auto count = grouping.get_region_count(file);
			for (std::size_t i = first; i < first + count; ++i)
			{
				REQUIRE(
					plan.get_region_file(grouping.get_region(i)) == file
				);
			}
		}
	}

	SECTION( "the regions of one file keep the order they were added in" )
	{
		const auto first = grouping.get_first_position(zero);
		REQUIRE( to_vector(
			plan.get_file_offset(grouping.get_region(first))
		) == std::vector<std::size_t>{3, 0, 0} );
		REQUIRE( to_vector(
			plan.get_file_offset(grouping.get_region(first + 1))
		) == std::vector<std::size_t>{9, 0, 0} );
	}

	SECTION( "both offsets of a region stay together" )
	{
		// Element 3 of stack_0 was added into slot 1 and element 9 into
		// slot 4. Grouping names regions rather than moving their offsets,
		// so one region's file offset can not end up beside another's
		// array offset.
		const auto first = grouping.get_first_position(zero);
		REQUIRE( to_vector(
			plan.get_array_offset(grouping.get_region(first))
		) == std::vector<std::size_t>{1, 0, 0} );
		REQUIRE( to_vector(
			plan.get_array_offset(grouping.get_region(first + 1))
		) == std::vector<std::size_t>{4, 0, 0} );
	}

	SECTION( "every region appears exactly once" )
	{
		std::vector<std::size_t> regions;
		for (std::size_t i = 0; i < grouping.get_size(); ++i)
		{
			regions.push_back(grouping.get_region(i));
		}
		std::sort(regions.begin(), regions.end());

		REQUIRE( regions == std::vector<std::size_t>{0, 1, 2, 3, 4} );
	}

	SECTION( "building again over the same plan changes nothing" )
	{
		grouping.build(plan);

		REQUIRE( grouping.get_size() == 5 );
		REQUIRE( grouping.get_first_position(one) == 2 );
		REQUIRE( grouping.get_region_count(one) == 2 );
	}

	SECTION( "building again picks up regions added since" )
	{
		add_element(plan, two, 5, 5);
		grouping.build(plan);

		REQUIRE( grouping.get_size() == 6 );
		REQUIRE( grouping.get_region_count(two) == 2 );
	}
}

TEST_CASE( "a file no region addresses is grouped as empty",
	"[region_grouping]" )
{
	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	const auto zero = plan.add_file("stack_0.mrcs");
	const auto empty = plan.add_file("stack_1.mrcs");
	add_element(plan, zero, 0, 0);

	region_grouping grouping;
	grouping.build(plan);

	REQUIRE( grouping.get_file_count() == 2 );
	REQUIRE( grouping.get_region_count(empty) == 0 );
	REQUIRE( grouping.get_first_position(empty) == 1 );
}

TEST_CASE( "an empty plan groups into nothing", "[region_grouping]" )
{
	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	plan.add_file("stack_0.mrcs");

	region_grouping grouping;
	grouping.build(plan);

	REQUIRE( grouping.get_size() == 0 );
	REQUIRE( grouping.get_file_count() == 1 );
	REQUIRE( grouping.get_region_count(0) == 0 );
	REQUIRE( grouping.get_first_position(0) == 0 );
}

TEST_CASE( "a region_grouping reused across transactions stops allocating",
	"[region_grouping]" )
{
	const std::size_t count = 64;

	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	plan.reserve(2, count);

	region_grouping grouping;
	grouping.reserve(2, count);

	const auto fill = [&] ()
	{
		plan.clear();
		const auto zero = plan.add_file("stack_0.mrcs");
		const auto one = plan.add_file("stack_1.mrcs");
		for (std::size_t i = 0; i < count; ++i)
		{
			add_element(plan, (i % 2) == 0 ? zero : one, i, i);
		}
		grouping.build(plan);
	};

	fill();
	const auto *offsets = plan.get_file_offset(0).data();

	for (std::size_t repeat = 0; repeat < 8; ++repeat)
	{
		fill();

		REQUIRE( grouping.get_size() == count );
		REQUIRE( grouping.get_region_count(0) == count / 2 );
		REQUIRE( grouping.get_region(0) == 0 );
		REQUIRE( grouping.get_region(count / 2) == 1 );
		REQUIRE( plan.get_file_offset(0).data() == offsets );
	}
}

TEST_CASE( "a region_grouping has value semantics", "[region_grouping]" )
{
	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	const auto file = plan.add_file("stack_0.mrcs");
	add_element(plan, file, 2, 0);

	region_grouping grouping;
	grouping.build(plan);

	SECTION( "a copy holds the same ordering" )
	{
		const region_grouping copy(grouping);

		REQUIRE( copy.get_size() == 1 );
		REQUIRE( copy.get_file_count() == 1 );
		REQUIRE( copy.get_region(0) == 0 );
	}

	SECTION( "a copy does not share storage with the one it copies" )
	{
		region_grouping copy(grouping);
		copy.clear();

		REQUIRE( copy.get_size() == 0 );
		REQUIRE( grouping.get_size() == 1 );
	}
}
