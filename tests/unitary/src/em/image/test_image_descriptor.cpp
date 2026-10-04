// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_descriptor.hpp>

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

const std::vector<std::size_t> stack_extents = {6, 4, 5};

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

image_descriptor make_descriptor(
	const std::vector<std::size_t> &extents,
	std::size_t core_rank,
	numerical_type data_type = numerical_type::float32
)
{
	return image_descriptor(make_span(extents), core_rank, data_type);
}

} // anonymous namespace

TEST_CASE(
	"image_descriptor holds the components it was constructed from",
	"[image_descriptor]"
)
{
	const auto descriptor =
		make_descriptor(stack_extents, 2, numerical_type::int16);

	CHECK( to_vector(descriptor.get_extents()) == stack_extents );
	CHECK( descriptor.get_core_rank() == 2 );
	CHECK( descriptor.get_data_type() == numerical_type::int16 );
}

TEST_CASE(
	"image_descriptor refuses what describes no image or volume",
	"[image_descriptor]"
)
{
	SECTION( "a core rank of zero" )
	{
		REQUIRE_THROWS_AS(
			make_descriptor(stack_extents, 0),
			std::invalid_argument
		);
	}

	SECTION( "a core rank above the rank of the extents" )
	{
		REQUIRE_THROWS_AS(
			make_descriptor(stack_extents, 4),
			std::invalid_argument
		);
	}

	SECTION( "no extents at all" )
	{
		REQUIRE_THROWS_AS(
			make_descriptor({}, 1),
			std::invalid_argument
		);
	}

	SECTION( "an unknown data type" )
	{
		REQUIRE_THROWS_AS(
			make_descriptor(stack_extents, 2, numerical_type::unknown),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"image_descriptor accepts a core rank spanning every extent",
	"[image_descriptor]"
)
{
	const auto descriptor = make_descriptor(stack_extents, 3);

	CHECK( descriptor.get_core_rank() == stack_extents.size() );
}

TEST_CASE( "image_descriptor has value semantics", "[image_descriptor]" )
{
	const auto descriptor = make_descriptor(stack_extents, 2);

	SECTION( "copies compare equal to their source" )
	{
		const image_descriptor copy(descriptor);

		CHECK( copy == descriptor );
		CHECK( to_vector(copy.get_extents()) == stack_extents );
	}

	SECTION( "assignment replaces every component" )
	{
		auto assigned = make_descriptor({8, 8, 8}, 3, numerical_type::int8);
		assigned = descriptor;

		CHECK( assigned == descriptor );
	}

	SECTION( "moves transfer every component" )
	{
		auto source = make_descriptor(stack_extents, 2);
		const image_descriptor moved(std::move(source));

		CHECK( moved == descriptor );
	}
}

TEST_CASE(
	"image_descriptor compares every component",
	"[image_descriptor]"
)
{
	const auto descriptor = make_descriptor(stack_extents, 2);

	SECTION( "equal components are equal" )
	{
		CHECK( descriptor == make_descriptor(stack_extents, 2) );
		CHECK_FALSE( descriptor != make_descriptor(stack_extents, 2) );
	}

	SECTION( "the extents take part" )
	{
		CHECK( descriptor != make_descriptor({6, 4, 4}, 2) );
	}

	SECTION( "the core rank tells a stack from a volume of the same extents" )
	{
		CHECK( descriptor != make_descriptor(stack_extents, 3) );
	}

	SECTION( "the data type takes part" )
	{
		CHECK(
			descriptor !=
			make_descriptor(stack_extents, 2, numerical_type::float64)
		);
	}
}

TEST_CASE( "image_descriptor hashes consistently", "[image_descriptor]" )
{
	const auto descriptor = make_descriptor(stack_extents, 2);

	SECTION( "equal descriptors hash equally" )
	{
		CHECK( descriptor.hash() == make_descriptor(stack_extents, 2).hash() );
		CHECK(
			std::hash<image_descriptor>()(descriptor) == descriptor.hash()
		);
	}

	SECTION( "the core rank takes part in the hash" )
	{
		CHECK( descriptor.hash() != make_descriptor(stack_extents, 3).hash() );
	}
}

TEST_CASE(
	"get_core_extents leaves out the axes a file stacks along",
	"[image_descriptor]"
)
{
	SECTION( "a stack of images keeps the shape of one image" )
	{
		const auto descriptor = make_descriptor(stack_extents, 2);

		CHECK(
			to_vector(get_core_extents(descriptor)) ==
			std::vector<std::size_t>{4, 5}
		);
	}

	SECTION( "a volume keeps every extent" )
	{
		const auto descriptor = make_descriptor(stack_extents, 3);

		CHECK( to_vector(get_core_extents(descriptor)) == stack_extents );
	}

	SECTION( "a stack of volumes keeps the shape of one volume" )
	{
		const auto descriptor = make_descriptor({2, 6, 4, 5}, 3);

		CHECK( to_vector(get_core_extents(descriptor)) == stack_extents );
	}
}
