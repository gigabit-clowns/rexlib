// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <rexlib/em/image/strict_image_transfer_sanitizer.hpp>

#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

const std::vector<std::size_t> image_extents = {100, 100};
const std::vector<std::size_t> patch_extents = {10, 10};
const std::vector<std::size_t> array_extents = {2, 10, 10};

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

void add_patch(
	image_transfer_plan &regions,
	std::size_t file_row,
	std::size_t file_column,
	std::size_t slot
)
{
	const std::size_t file_offset[2] = {file_row, file_column};
	const std::size_t array_offset[3] = {slot, 0, 0};
	regions.add(make_span(file_offset, 2), make_span(array_offset, 3));
}

std::vector<image_transfer_plan> sanitize(const image_transfer_plan &regions)
{
	return strict_image_transfer_sanitizer::get_shared()->sanitize(
		regions,
		make_span(image_extents),
		make_span(array_extents)
	);
}

} // anonymous namespace

TEST_CASE(
	"strict_image_transfer_sanitizer answers regions that fit with the plan "
	"it was shown",
	"[strict_image_transfer_sanitizer]"
)
{
	image_transfer_plan regions(image_transfer_shape(patch_extents, 2, 3));

	SECTION( "regions that fit both sides" )
	{
		// The last patch ends exactly at the far edge of the image, which
		// fits.
		add_patch(regions, 0, 0, 0);
		add_patch(regions, 90, 90, 1);

		const auto result = sanitize(regions);

		REQUIRE( result.size() == 1 );
		CHECK( to_vector(result[0].get_shape().get_extents()) ==
			patch_extents );
		REQUIRE( result[0].get_region_count() == 2 );
		CHECK( to_vector(result[0].get_file_offset(1)) ==
			std::vector<std::size_t>{90, 90} );
		CHECK( to_vector(result[0].get_array_offset(1)) ==
			std::vector<std::size_t>{1, 0, 0} );
	}

	SECTION( "no region at all" )
	{
		const auto result = sanitize(regions);

		REQUIRE( result.size() == 1 );
		CHECK( result[0].get_region_count() == 0 );
	}
}

TEST_CASE(
	"strict_image_transfer_sanitizer refuses a region that does not fit, "
	"naming the side",
	"[strict_image_transfer_sanitizer]"
)
{
	image_transfer_plan regions(image_transfer_shape(patch_extents, 2, 3));
	add_patch(regions, 0, 0, 0);

	SECTION( "one running off the file" )
	{
		add_patch(regions, 95, 93, 1);

		REQUIRE_THROWS_MATCHES(
			sanitize(regions),
			std::out_of_range,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::ContainsSubstring("file")
			)
		);
	}

	SECTION( "one starting past the end of the file" )
	{
		add_patch(regions, 101, 0, 1);

		REQUIRE_THROWS_AS( sanitize(regions), std::out_of_range );
	}

	SECTION( "one whose slot is not there" )
	{
		// The batch axis is a leading axis, which spans a single position.
		add_patch(regions, 0, 0, 2);

		REQUIRE_THROWS_MATCHES(
			sanitize(regions),
			std::out_of_range,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::ContainsSubstring("array")
			)
		);
	}

	SECTION( "one the array offset pushes over the end of its slot" )
	{
		const std::size_t file_offset[2] = {0, 0};
		const std::size_t array_offset[3] = {1, 4, 0};
		regions.add(make_span(file_offset, 2), make_span(array_offset, 3));

		REQUIRE_THROWS_AS( sanitize(regions), std::out_of_range );
	}
}

TEST_CASE(
	"strict_image_transfer_sanitizer checks the rank of each side",
	"[strict_image_transfer_sanitizer]"
)
{
	const image_transfer_plan regions(
		image_transfer_shape(patch_extents, 2, 3)
	);
	const auto &sanitizer = *strict_image_transfer_sanitizer::get_shared();

	SECTION( "file extents that do not have the file rank" )
	{
		REQUIRE_THROWS_AS(
			sanitizer.sanitize(
				regions,
				make_span(array_extents),
				make_span(array_extents)
			),
			std::invalid_argument
		);
	}

	SECTION( "array extents that do not have the array rank" )
	{
		REQUIRE_THROWS_AS(
			sanitizer.sanitize(
				regions,
				make_span(image_extents),
				make_span(image_extents)
			),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"strict_image_transfer_sanitizer has one instance every use shares",
	"[strict_image_transfer_sanitizer]"
)
{
	const auto first = strict_image_transfer_sanitizer::get_shared();

	REQUIRE( first != nullptr );
	CHECK( first == strict_image_transfer_sanitizer::get_shared() );
}
