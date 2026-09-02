// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_read_format_manager.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/service_catalog.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_probe.hpp>

#include <cstddef>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE( "the image format managers are reachable as services",
	"[image_format_manager]" )
{
	service_catalog catalog;

	SECTION( "the read manager is served by the catalog" )
	{
		const auto manager =
			catalog.get_service_manager<image_read_format_manager>();

		REQUIRE( manager != nullptr );
	}

	SECTION( "the write manager is served by the catalog" )
	{
		const auto manager =
			catalog.get_service_manager<image_write_format_manager>();

		REQUIRE( manager != nullptr );
	}

	SECTION( "asking twice yields the same manager" )
	{
		const auto first =
			catalog.get_service_manager<image_read_format_manager>();
		const auto second =
			catalog.get_service_manager<image_read_format_manager>();

		REQUIRE( first == second );
	}

	SECTION( "reading and writing are separate services" )
	{
		const auto reading =
			catalog.get_service_manager<image_read_format_manager>();
		const auto writing =
			catalog.get_service_manager<image_write_format_manager>();

		REQUIRE(
			static_cast<const void*>(reading.get()) !=
			static_cast<const void*>(writing.get())
		);
	}
}

TEST_CASE( "the bundled library registers no image format yet",
	"[image_format_manager]" )
{
	service_catalog catalog;
	const image_probe probe("absent.mrc");

	SECTION( "no format claims a file for reading" )
	{
		const auto manager =
			catalog.get_service_manager<image_read_format_manager>();

		REQUIRE( manager->get_most_suitable_format(probe) == nullptr );
		REQUIRE_THROWS_AS(
			manager->open("absent.mrc"),
			invalid_operation_error
		);
	}

	SECTION( "no format claims a file for writing" )
	{
		const auto manager =
			catalog.get_service_manager<image_write_format_manager>();
		const std::vector<std::size_t> extents = {2, 2};

		REQUIRE( manager->get_most_suitable_format(probe) == nullptr );
		REQUIRE_THROWS_AS(
			manager->open(
				"absent.mrc",
				make_span(extents),
				numerical_type::float32,
				image_metadata()
			),
			invalid_operation_error
		);
	}
}
