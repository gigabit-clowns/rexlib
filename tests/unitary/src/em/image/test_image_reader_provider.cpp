// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_reader_provider.hpp>

#include "mock/mock_image_reader.hpp"
#include "mock/mock_image_reader_provider.hpp"

#include <rexlib/core/exceptions/invalid_operation_error.hpp>

#include <cstddef>
#include <memory>
#include <trompeloeil.hpp>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// A stack of six planes of three by five: the leading axis is the one it
// stacks along, so one image of it is the trailing two extents.
const std::vector<std::size_t> stack_extents = {6, 3, 5};
const std::vector<std::size_t> plane_extents = {3, 5};

// A volume stacks along nothing, so its core rank is its whole rank.
const std::vector<std::size_t> volume_extents = {20, 20, 20};

} // namespace

TEST_CASE(
	"a query answers the shape of a file a provider serves",
	"[image_reader_provider]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(stack_extents));
	ALLOW_CALL(*reader, get_core_rank()).RETURN(plane_extents.size());
	REQUIRE_CALL(*readers, acquire("stack.mrcs")).RETURN(reader);

	SECTION( "every extent of the file" )
	{
		CHECK( query_extents(*readers, "stack.mrcs") == stack_extents );
	}

	SECTION( "the extents of one image of it" )
	{
		CHECK( query_core_extents(*readers, "stack.mrcs") == plane_extents );
	}
}

TEST_CASE(
	"a query leaves nothing out of a file that stacks along nothing",
	"[image_reader_provider]"
)
{
	// A volume is one image of itself.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(volume_extents));
	ALLOW_CALL(*reader, get_core_rank()).RETURN(volume_extents.size());
	REQUIRE_CALL(*readers, acquire("volume.mrc")).RETURN(reader);

	CHECK( query_core_extents(*readers, "volume.mrc") == volume_extents );
}

TEST_CASE(
	"a query reports what acquiring the file reported",
	"[image_reader_provider]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();

	REQUIRE_CALL(*readers, acquire("absent.mrc"))
		.SIDE_EFFECT( throw invalid_operation_error("nothing claims it") )
		.RETURN(nullptr);

	REQUIRE_THROWS_AS(
		query_extents(*readers, "absent.mrc"),
		invalid_operation_error
	);
}
