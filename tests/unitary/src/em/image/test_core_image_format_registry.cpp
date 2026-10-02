// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/core_image_format_registry.hpp>

#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_read_format.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>
#include <rexlib/em/image/image_write_format.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>
#include <rexlib/tests/assets.hpp>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE(
	"each core image format registry is one instance",
	"[core_image_format_registry]"
)
{
	REQUIRE( &get_core_image_read_format_registry() ==
		&get_core_image_read_format_registry() );
	REQUIRE( &get_core_image_write_format_registry() ==
		&get_core_image_write_format_registry() );
}

TEST_CASE(
	"the core image format registries hold the bundled formats",
	"[core_image_format_registry]"
)
{
	SECTION( "draining the read registry lets a manager read an MRC file" )
	{
		image_read_format_manager manager;
		get_core_image_read_format_registry().register_all(manager);

		const auto *chosen = manager.get_most_suitable_format(
			image_probe(get_mrc_asset_path("EMD-3197.map")));

		REQUIRE( chosen != nullptr );
		REQUIRE( chosen->get_name() == "MRC" );
	}

	SECTION( "draining the write registry lets a manager create an MRC file" )
	{
		image_write_format_manager manager;
		get_core_image_write_format_registry().register_all(manager);

		const auto *chosen = manager.get_most_suitable_format(
			image_probe("absent.mrc"));

		REQUIRE( chosen != nullptr );
		REQUIRE( chosen->get_name() == "MRC" );
	}
}
