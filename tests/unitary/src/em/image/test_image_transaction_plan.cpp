// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_transaction_plan.hpp>

#include <algorithm>
#include <cstddef>
#include <stdexcept>
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

TEST_CASE( "an image_transaction_plan starts empty of files and regions",
	"[image_transaction_plan]" )
{
	const image_transaction_plan plan(make_span(plane_extents), 3, 3);

	REQUIRE( plan.get_size() == 0 );
	REQUIRE( plan.get_file_count() == 0 );
	REQUIRE( to_vector(plan.get_extents()) == plane_extents );
	REQUIRE( plan.get_rank() == 2 );
	REQUIRE( plan.get_file_rank() == 3 );
	REQUIRE( plan.get_array_rank() == 3 );
}

TEST_CASE( "an image_transaction_plan states the shape of its regions",
	"[image_transaction_plan]" )
{
	SECTION( "the shape is stated when the plan is constructed" )
	{
		const image_transaction_plan plan(make_span(plane_extents), 3, 3);

		REQUIRE( to_vector(plan.get_extents()) == plane_extents );
		REQUIRE( plan.get_rank() == 2 );
		REQUIRE( plan.get_file_rank() == 3 );
		REQUIRE( plan.get_array_rank() == 3 );
		REQUIRE( plan.get_size() == 0 );
	}

	SECTION( "the two sides may differ in rank" )
	{
		// Patches cut out of two dimensional micrographs into a three
		// dimensional array.
		const std::vector<std::size_t> extents = {4, 4};
		const image_transaction_plan plan(make_span(extents), 2, 3);

		REQUIRE( plan.get_file_rank() == 2 );
		REQUIRE( plan.get_array_rank() == 3 );
	}

	SECTION( "a region that does not fit the file rank is refused" )
	{
		const std::vector<std::size_t> extents = {1, 4, 4};

		REQUIRE_THROWS_AS(
			image_transaction_plan(make_span(extents), 2, 3),
			std::invalid_argument
		);
	}

	SECTION( "a region that does not fit the array rank is refused" )
	{
		const std::vector<std::size_t> extents = {1, 4, 4};

		REQUIRE_THROWS_AS(
			image_transaction_plan(make_span(extents), 3, 2),
			std::invalid_argument
		);
	}
}

TEST_CASE( "an image_transaction_plan names each file once",
	"[image_transaction_plan]" )
{
	image_transaction_plan plan(make_span(plane_extents), 3, 3);

	SECTION( "a path named twice keeps the index it was first given" )
	{
		const auto first = plan.add_file("stack_0.mrcs");
		const auto again = plan.add_file("stack_0.mrcs");

		REQUIRE( first == again );
		REQUIRE( plan.get_file_count() == 1 );
	}

	SECTION( "distinct paths get distinct indices" )
	{
		const auto first = plan.add_file("stack_0.mrcs");
		const auto second = plan.add_file("stack_1.mrcs");

		REQUIRE( plan.get_file_count() == 2 );
		REQUIRE( plan.get_file(first) == "stack_0.mrcs" );
		REQUIRE( plan.get_file(second) == "stack_1.mrcs" );
	}
}

TEST_CASE( "an image_transaction_plan does not care how big its files are",
	"[image_transaction_plan]" )
{
	// The stacks of a dataset are not the same size, and nothing here may
	// require otherwise: a plan constrains the rank of its files and records
	// the extents of none of them. Element 999 of one stack and element 12
	// of another belong to one transaction like any other, and only the
	// reader of each file is in a position to say whether they exist.
	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	const auto small = plan.add_file("small.mrcs");
	const auto large = plan.add_file("large.mrcs");

	add_element(plan, large, 1199, 0);
	add_element(plan, small, 12, 1);
	add_element(plan, large, 999, 2);

	REQUIRE( plan.get_size() == 3 );
	REQUIRE( to_vector(plan.get_file_offset(0)) ==
		std::vector<std::size_t>{1199, 0, 0} );
	REQUIRE( to_vector(plan.get_file_offset(1)) ==
		std::vector<std::size_t>{12, 0, 0} );
}

TEST_CASE( "an image_transaction_plan refuses a region it can not hold",
	"[image_transaction_plan]" )
{
	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	const auto file = plan.add_file("stack_0.mrcs");

	const std::size_t two[2] = {0, 0};
	const std::size_t three[3] = {0, 0, 0};

	SECTION( "a file index naming no file is refused" )
	{
		REQUIRE_THROWS_AS(
			plan.add(file + 1, make_span(three, 3), make_span(three, 3)),
			std::out_of_range
		);
	}

	SECTION( "the file offset must have the file rank" )
	{
		REQUIRE_THROWS_AS(
			plan.add(file, make_span(two, 2), make_span(three, 3)),
			std::invalid_argument
		);
	}

	SECTION( "the array offset must have the array rank" )
	{
		REQUIRE_THROWS_AS(
			plan.add(file, make_span(three, 3), make_span(two, 2)),
			std::invalid_argument
		);
	}

	SECTION( "a refused region is not appended" )
	{
		REQUIRE_THROWS_AS(
			plan.add(file, make_span(two, 2), make_span(three, 3)),
			std::invalid_argument
		);
		REQUIRE_THROWS_AS(
			plan.add(file, make_span(three, 3), make_span(two, 2)),
			std::invalid_argument
		);
		REQUIRE( plan.get_size() == 0 );
	}
}

TEST_CASE( "clearing an image_transaction_plan keeps the shape",
	"[image_transaction_plan]" )
{
	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	const auto file = plan.add_file("stack_0.mrcs");
	add_element(plan, file, 0, 0);
	plan.clear();

	REQUIRE( plan.get_size() == 0 );
	REQUIRE( plan.get_file_count() == 0 );
	REQUIRE( to_vector(plan.get_extents()) == plane_extents );
	REQUIRE( plan.get_file_rank() == 3 );
	REQUIRE( plan.get_array_rank() == 3 );
}

TEST_CASE( "an image_transaction_plan reused across calls stops allocating",
	"[image_transaction_plan]" )
{
	const std::size_t count = 64;

	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	plan.reserve(2, count);

	const auto fill = [&] ()
	{
		plan.clear();
		const auto zero = plan.add_file("stack_0.mrcs");
		const auto one = plan.add_file("stack_1.mrcs");
		for (std::size_t i = 0; i < count; ++i)
		{
			add_element(plan, (i % 2) == 0 ? zero : one, i, i);
		}
	};

	fill();
	const auto *offsets = plan.get_file_offset(0).data();

	for (std::size_t repeat = 0; repeat < 8; ++repeat)
	{
		fill();

		REQUIRE( plan.get_size() == count );
		REQUIRE( plan.get_file_count() == 2 );
		REQUIRE( plan.get_file_offset(0).data() == offsets );
	}
}

TEST_CASE( "an image_transaction_plan has value semantics",
	"[image_transaction_plan]" )
{
	image_transaction_plan plan(make_span(plane_extents), 3, 3);
	const auto file = plan.add_file("stack_0.mrcs");
	add_element(plan, file, 2, 0);

	SECTION( "a copy holds the same files and regions" )
	{
		const image_transaction_plan copy(plan);

		REQUIRE( copy.get_size() == 1 );
		REQUIRE( copy.get_file_count() == 1 );
		REQUIRE( copy.get_file(0) == "stack_0.mrcs" );
		REQUIRE( to_vector(copy.get_file_offset(0)) ==
			std::vector<std::size_t>{2, 0, 0} );
		REQUIRE( copy.get_region_file(0) == 0 );
	}

	SECTION( "a copy does not share storage with the plan it copies" )
	{
		image_transaction_plan copy(plan);
		copy.clear();

		REQUIRE( copy.get_size() == 0 );
		REQUIRE( plan.get_size() == 1 );
	}
}
