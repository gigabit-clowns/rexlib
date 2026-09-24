// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_read_format_manager.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/service_catalog.hpp>
#include <rexlib/em/image/exceptions/image_file_error.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
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

TEST_CASE( "a file that is not there is claimed by no bundled format",
	"[image_format_manager]" )
{
	service_catalog catalog;
	const image_probe probe("absent.mrc");

	SECTION( "no format claims a file for reading" )
	{
		// The bundled MRC format reads a file on the identifier its header
		// carries, and a file that is not there carries none. What it does
		// claim is pinned by test_mrc_real_files.cpp.
		const auto manager =
			catalog.get_service_manager<image_read_format_manager>();

		REQUIRE( manager->get_most_suitable_format(probe) == nullptr );
		REQUIRE_THROWS_AS(
			manager->open("absent.mrc"),
			image_file_error
		);
	}

	SECTION( "the MRC format claims it for writing" )
	{
		// A file being created does not exist yet, so the extension is the
		// whole of what a write format has to decide on.
		const auto manager =
			catalog.get_service_manager<image_write_format_manager>();

		REQUIRE( manager->get_most_suitable_format(probe) != nullptr );
	}

	SECTION( "an extension no format writes is still claimed by none" )
	{
		const auto manager =
			catalog.get_service_manager<image_write_format_manager>();
		const image_probe other("absent.eer");
		const std::vector<std::size_t> extents = {2, 2};
		const image_descriptor descriptor(
			make_span(extents),
			2,
			numerical_type::float32
		);

		REQUIRE( manager->get_most_suitable_format(other) == nullptr );
		REQUIRE_THROWS_AS(
			manager->open("absent.eer", descriptor, image_metadata()),
			unsupported_operation_error
		);
	}
}
