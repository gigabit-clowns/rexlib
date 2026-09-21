// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_reader.hpp>

#include "mock/mock_image_reader.hpp"

#include <cstddef>
#include <trompeloeil.hpp>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// A stack of six images of three by five: the leading axis is the one it
// stacks along, so one image of it is the trailing two extents.
const std::vector<std::size_t> stack_extents = {6, 3, 5};
const std::vector<std::size_t> plane_extents = {3, 5};

// A volume stacks along nothing, so its core rank is its whole rank.
const std::vector<std::size_t> volume_extents = {20, 20, 20};

} // namespace

TEST_CASE( "copy_extents owns every extent of a file", "[image_reader]" )
{
	// The point of copying: what a reader reports refers to storage it owns,
	// and the result must outlive the reader it came from.
	mock_image_reader reader;
	ALLOW_CALL(reader, get_extents()).RETURN(make_span(stack_extents));

	const auto extents = copy_extents(reader);

	CHECK( extents == stack_extents );
	CHECK( extents.data() != stack_extents.data() );
}

TEST_CASE(
	"copy_core_extents leaves out the axes a file stacks along",
	"[image_reader]"
)
{
	mock_image_reader reader;

	SECTION( "a stack keeps the shape of one image of it" )
	{
		ALLOW_CALL(reader, get_extents()).RETURN(make_span(stack_extents));
		ALLOW_CALL(reader, get_core_rank()).RETURN(plane_extents.size());

		CHECK( copy_core_extents(reader) == plane_extents );
	}

	SECTION( "a volume is one image of itself and keeps all of them" )
	{
		ALLOW_CALL(reader, get_extents()).RETURN(make_span(volume_extents));
		ALLOW_CALL(reader, get_core_rank()).RETURN(volume_extents.size());

		CHECK( copy_core_extents(reader) == volume_extents );
	}
}
