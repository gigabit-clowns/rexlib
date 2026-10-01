// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <rexlib/em/image/image_transfer_shape.hpp>

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

} // anonymous namespace

TEST_CASE(
	"an image_transfer_shape holds the extents and the ranks of both sides",
	"[image_transfer_shape]"
)
{
	SECTION( "sides of higher rank than the extents" )
	{
		const image_transfer_shape shape(plane_extents, 3, 4);

		CHECK( to_vector(shape.get_extents()) == plane_extents );
		CHECK( shape.get_rank() == 2 );
		CHECK( shape.get_file_rank() == 3 );
		CHECK( shape.get_array_rank() == 4 );
	}

	SECTION( "the array side may outrank the file side" )
	{
		const image_transfer_shape shape(plane_extents, 2, 3);

		CHECK( shape.get_file_rank() == 2 );
		CHECK( shape.get_array_rank() == 3 );
	}

	SECTION( "the file side may outrank the array side" )
	{
		const image_transfer_shape shape(plane_extents, 3, 2);

		CHECK( shape.get_file_rank() == 3 );
		CHECK( shape.get_array_rank() == 2 );
	}

	SECTION( "sides of exactly the rank of the extents" )
	{
		const image_transfer_shape shape(plane_extents, 2, 2);

		CHECK( shape.get_rank() == 2 );
		CHECK( shape.get_file_rank() == 2 );
		CHECK( shape.get_array_rank() == 2 );
	}

	SECTION( "no extents at all, which outrank nothing" )
	{
		const image_transfer_shape shape(std::vector<std::size_t>(), 0, 0);

		CHECK( shape.get_extents().empty() );
		CHECK( shape.get_rank() == 0 );
	}
}

TEST_CASE(
	"an image_transfer_shape refuses extents outranking a side, naming it",
	"[image_transfer_shape]"
)
{
	const std::vector<std::size_t> extents = {1, 4, 4};

	SECTION( "the file side" )
	{
		REQUIRE_THROWS_MATCHES(
			image_transfer_shape(extents, 2, 3),
			std::invalid_argument,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith("image_transfer_shape: ") &&
				Catch::Matchers::ContainsSubstring("file")
			)
		);
	}

	SECTION( "the array side" )
	{
		REQUIRE_THROWS_MATCHES(
			image_transfer_shape(extents, 3, 2),
			std::invalid_argument,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith("image_transfer_shape: ") &&
				Catch::Matchers::ContainsSubstring("array")
			)
		);
	}
}

TEST_CASE(
	"an image_transfer_shape counts the leading axes its extents do not "
	"reach",
	"[image_transfer_shape]"
)
{
	const image_transfer_shape shape(plane_extents, 2, 4);

	SECTION( "a side of the rank of the extents has none" )
	{
		CHECK( shape.get_leading_rank(shape.get_file_rank()) == 0 );
	}

	SECTION( "a side of higher rank has the difference" )
	{
		CHECK( shape.get_leading_rank(shape.get_array_rank()) == 2 );
	}

	SECTION( "every axis is a leading one when there are no extents" )
	{
		const image_transfer_shape point(std::vector<std::size_t>(), 3, 3);

		CHECK( point.get_leading_rank(3) == 3 );
	}
}

TEST_CASE(
	"an image_transfer_shape resolves the extent along each axis of a side",
	"[image_transfer_shape]"
)
{
	const image_transfer_shape shape(plane_extents, 2, 4);

	SECTION( "a side of the rank of the extents spans them all" )
	{
		CHECK( shape.get_extent(2, 0) == 3 );
		CHECK( shape.get_extent(2, 1) == 5 );
	}

	SECTION( "a side of higher rank spans one along its leading axes" )
	{
		CHECK( shape.get_extent(4, 0) == 1 );
		CHECK( shape.get_extent(4, 1) == 1 );
		CHECK( shape.get_extent(4, 2) == 3 );
		CHECK( shape.get_extent(4, 3) == 5 );
	}

	SECTION( "a side spans one along every axis when there are no extents" )
	{
		const image_transfer_shape point(std::vector<std::size_t>(), 2, 2);

		CHECK( point.get_extent(2, 0) == 1 );
		CHECK( point.get_extent(2, 1) == 1 );
	}
}

TEST_CASE(
	"an image_transfer_shape has value semantics",
	"[image_transfer_shape]"
)
{
	const image_transfer_shape shape(plane_extents, 3, 4);

	SECTION( "a copy states what the original states" )
	{
		const image_transfer_shape copy(shape);

		CHECK( to_vector(copy.get_extents()) == plane_extents );
		CHECK( copy.get_file_rank() == 3 );
		CHECK( copy.get_array_rank() == 4 );
	}

	SECTION( "assigning replaces what a shape states" )
	{
		image_transfer_shape other(std::vector<std::size_t>{7}, 1, 1);

		other = shape;

		CHECK( to_vector(other.get_extents()) == plane_extents );
		CHECK( other.get_file_rank() == 3 );
		CHECK( other.get_array_rank() == 4 );
	}
}
