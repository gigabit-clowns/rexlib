// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_metadata.hpp>

#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

std::vector<double> to_vector(span<const double> values)
{
	return std::vector<double>(values.begin(), values.end());
}

} // namespace

TEST_CASE( "image_metadata distinguishes unstated from zero",
	"[image_metadata]" )
{
	SECTION( "a default instance states nothing" )
	{
		const image_metadata metadata;

		REQUIRE( metadata.get_sampling().empty() );
	}

	SECTION( "a stated sampling is returned per axis" )
	{
		const std::vector<double> expected = {1.0, 1.34, 1.34};
		const image_metadata metadata(expected);

		REQUIRE( to_vector(metadata.get_sampling()) == expected );
	}

	SECTION( "a sampling of one is not the same as an unstated sampling" )
	{
		const std::vector<double> ones = {1.0, 1.0};
		const image_metadata stated(ones);
		const image_metadata unstated;

		REQUIRE( to_vector(stated.get_sampling()) == ones );
		REQUIRE( unstated.get_sampling().empty() );
	}

	SECTION( "the sampling is viewed rather than copied out" )
	{
		const std::vector<double> expected = {2.0, 3.0};
		const image_metadata metadata(expected);

		REQUIRE( metadata.get_sampling().data() ==
			metadata.get_sampling().data() );
		REQUIRE( metadata.get_sampling().size() == 2 );
	}
}

TEST_CASE( "image_metadata has value semantics", "[image_metadata]" )
{
	const std::vector<double> expected = {1.0, 1.34, 1.34};
	const image_metadata metadata(expected);

	SECTION( "a copy holds the same sampling" )
	{
		const image_metadata copy(metadata);

		REQUIRE( to_vector(copy.get_sampling()) == expected );
	}

	SECTION( "a copy does not share storage with its source" )
	{
		const image_metadata copy(metadata);

		REQUIRE( copy.get_sampling().data() !=
			metadata.get_sampling().data() );
	}
}
