// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/memory_mapping/image_file_layout.hpp>

#include <rexlib/core/memory/byte_order.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>
#include <rexlib/em/image/image_descriptor.hpp>

#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

const std::vector<std::size_t> stack_extents = {5, 3, 4};
const std::vector<std::ptrdiff_t> stack_strides = {12, 4, 1};

image_descriptor make_descriptor(
	numerical_type data_type = numerical_type::float32
)
{
	return image_descriptor(make_span(stack_extents), 2, data_type);
}

} // anonymous namespace

TEST_CASE(
	"an image_file_layout holds what it was constructed with",
	"[image_file_layout]"
)
{
	const image_file_layout layout(
		make_descriptor(),
		stack_strides,
		1024,
		byte_order::big_endian
	);

	const auto strides = layout.get_strides();

	CHECK( layout.get_descriptor() == make_descriptor() );
	CHECK( std::vector<std::ptrdiff_t>(strides.begin(), strides.end()) ==
		stack_strides );
	CHECK( layout.get_data_offset() == 1024 );
	CHECK( layout.get_byte_order() == byte_order::big_endian );
}

TEST_CASE(
	"an image_file_layout reports how many bytes its values occupy",
	"[image_file_layout]"
)
{
	SECTION( "elements of four bytes" )
	{
		const image_file_layout layout(
			make_descriptor(),
			stack_strides,
			0,
			byte_order::little_endian
		);

		CHECK( layout.get_data_size() == 60 * 4 );
	}

	SECTION( "elements of one byte" )
	{
		const image_file_layout layout(
			make_descriptor(numerical_type::uint8),
			stack_strides,
			0,
			byte_order::little_endian
		);

		CHECK( layout.get_data_size() == 60 );
	}
}

TEST_CASE(
	"an image_file_layout refuses strides that do not have the rank of its "
	"extents",
	"[image_file_layout]"
)
{
	REQUIRE_THROWS_AS(
		image_file_layout(
			make_descriptor(),
			std::vector<std::ptrdiff_t>{4, 1},
			0,
			byte_order::little_endian
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"an image_file_layout refuses values that could not be addressed where "
	"they begin",
	"[image_file_layout]"
)
{
	SECTION( "an offset that misaligns them is refused" )
	{
		REQUIRE_THROWS_AS(
			image_file_layout(
				make_descriptor(),
				stack_strides,
				1026,
				byte_order::little_endian
			),
			image_format_error
		);
	}

	SECTION( "one that keeps them aligned is not" )
	{
		REQUIRE_NOTHROW(
			image_file_layout(
				make_descriptor(),
				stack_strides,
				1028,
				byte_order::little_endian
			)
		);
	}

	SECTION( "a narrower element tolerates a smaller multiple" )
	{
		REQUIRE_NOTHROW(
			image_file_layout(
				make_descriptor(numerical_type::int8),
				stack_strides,
				1027,
				byte_order::little_endian
			)
		);
	}
}
