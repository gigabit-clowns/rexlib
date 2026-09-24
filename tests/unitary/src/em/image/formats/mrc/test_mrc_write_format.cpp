// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/mrc/mrc_write_format.hpp>

#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_writer.hpp>

#include "../../fixtures/scoped_path.hpp"

#include <cstddef>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::mrc;

TEST_CASE( "the MRC format claims the files it can create",
	"[mrc_write_format]" )
{
	const mrc_write_format format;

	SECTION( "it is named" )
	{
		REQUIRE( format.get_name() == "MRC" );
	}

	SECTION( "the extensions it creates are claimed" )
	{
		REQUIRE( format.get_suitability(image_probe("absent.mrc")) ==
			backend_priority::normal );
		REQUIRE( format.get_suitability(image_probe("absent.mrcs")) ==
			backend_priority::normal );
		REQUIRE( format.get_suitability(image_probe("absent.map")) ==
			backend_priority::normal );
	}

	SECTION( "an extension it reads but does not create is not claimed" )
	{
		REQUIRE( format.get_suitability(image_probe("absent.st")) ==
			backend_priority::unsupported );
		REQUIRE( format.get_suitability(image_probe("absent.rec")) ==
			backend_priority::unsupported );
	}

	SECTION( "any other extension is not claimed" )
	{
		REQUIRE( format.get_suitability(image_probe("absent.tif")) ==
			backend_priority::unsupported );
		REQUIRE( format.get_suitability(image_probe("absent")) ==
			backend_priority::unsupported );
	}

	SECTION( "a claimed file opens into a writer" )
	{
		const scoped_path path("writer_claimed.mrc");
		const std::vector<std::size_t> extents = {3, 4};
		const image_descriptor descriptor(
			make_span(extents),
			2,
			numerical_type::float32
		);

		const auto writer = format.open(
			image_probe(path.get()),
			descriptor,
			image_metadata()
		);

		REQUIRE( writer != nullptr );
		REQUIRE( writer->get_descriptor() == descriptor );
	}
}
