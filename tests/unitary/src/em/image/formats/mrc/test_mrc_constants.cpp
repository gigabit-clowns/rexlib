// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/mrc/mrc_constants.hpp>

using namespace rexlib::em::mrc;

TEST_CASE(
	"is_volume_stack_space_group recognizes the range of stacks of volumes",
	"[mrc_constants]"
)
{
	SECTION( "both ends of the range are in it" )
	{
		REQUIRE(
			is_volume_stack_space_group(first_volume_stack_space_group)
		);
		REQUIRE(
			is_volume_stack_space_group(last_volume_stack_space_group)
		);
	}

	SECTION( "the space groups just outside the range are not" )
	{
		REQUIRE_FALSE(
			is_volume_stack_space_group(first_volume_stack_space_group - 1)
		);
		REQUIRE_FALSE(
			is_volume_stack_space_group(last_volume_stack_space_group + 1)
		);
	}

	SECTION( "an image stack and a single volume are not stacks of volumes" )
	{
		REQUIRE_FALSE( is_volume_stack_space_group(image_stack_space_group) );
		REQUIRE_FALSE( is_volume_stack_space_group(volume_space_group) );
	}
}
