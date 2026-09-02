// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_read_format_manager.hpp>

#include "fake/fake_registered_format.hpp"

#include <rexlib/em/image/image_probe.hpp>

#include <string>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE( "a format registers itself at static initialization",
	"[image_format_registration]" )
{
	image_read_format_manager manager;

	SECTION( "it is absent until the builtin formats are drained" )
	{
		REQUIRE( manager.get_most_suitable_format(
			image_probe("absent.fake")) == nullptr );
	}

	SECTION( "draining the builtin formats reaches it" )
	{
		manager.register_builtin_backends();

		const auto *chosen = manager.get_most_suitable_format(
			image_probe("absent.fake"));

		REQUIRE( chosen != nullptr );
		REQUIRE( chosen->get_name() == fake_registered_format_name );
	}

	SECTION( "it declines a file it does not recognize" )
	{
		manager.register_builtin_backends();

		REQUIRE( manager.get_most_suitable_format(
			image_probe("absent.mrc")) == nullptr );
	}

	SECTION( "the drained format opens a reader" )
	{
		manager.register_builtin_backends();

		const auto reader = manager.open("absent.fake");

		REQUIRE( reader != nullptr );
		REQUIRE( reader->get_data_type() == numerical_type::int16 );
	}
}
