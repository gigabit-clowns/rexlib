// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/tiff/tiff_read_format.hpp>

#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_reader.hpp>

#include "../../fixtures/scoped_path.hpp"
#include "fixtures/host_array.hpp"
#include "fixtures/tiff_test_file.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::tiff;
using namespace rexlib::test;

namespace
{

void write_image(const std::string &path, const char *mode)
{
	write_striped_file<std::uint8_t>(
		path, mode, 4, 3, SAMPLEFORMAT_UINT, COMPRESSION_LZW, 3,
		{counting<std::uint8_t>(12)}
	);
}

} // anonymous namespace

TEST_CASE( "the TIFF format claims the files it can read",
	"[tiff_read_format]" )
{
	const scoped_path path("tiff_read_format_claimed.tif");
	const tiff_read_format format;

	SECTION( "it is named" )
	{
		REQUIRE( format.get_name() == "TIFF" );
	}

	SECTION( "a classic file of either byte order is claimed" )
	{
		write_image(path.get(), "wl");

		REQUIRE( format.get_suitability(image_probe(path.get())) ==
			backend_priority::normal );

		write_image(path.get(), "wb");

		REQUIRE( format.get_suitability(image_probe(path.get())) ==
			backend_priority::normal );
	}

	SECTION( "a BigTIFF file is claimed" )
	{
		write_image(path.get(), "w8");

		REQUIRE( format.get_suitability(image_probe(path.get())) ==
			backend_priority::normal );
	}

	SECTION( "the signature is claimed whatever the extension" )
	{
		const std::string other = path.get() + ".unknown";
		write_image(other, "w");

		REQUIRE( format.get_suitability(image_probe(other)) ==
			backend_priority::normal );

		std::remove(other.c_str());
	}

	SECTION( "a file of the extension that holds something else is not" )
	{
		write_file(path.get(), std::vector<char>(64, 'x'));

		REQUIRE( format.get_suitability(image_probe(path.get())) ==
			backend_priority::unsupported );
	}

	SECTION( "a file that is not there is not claimed" )
	{
		REQUIRE( format.get_suitability(image_probe(path.get())) ==
			backend_priority::unsupported );
	}
}

TEST_CASE( "the TIFF read format opens a file as its pages state it",
	"[tiff_read_format]" )
{
	const scoped_path path("tiff_read_format_opened.tif");
	const tiff_read_format format;

	SECTION( "a claimed file opens into a reader of its values" )
	{
		write_image(path.get(), "w");

		const auto reader = format.open(image_probe(path.get()));

		const std::vector<std::size_t> extents = {3, 4};

		REQUIRE( reader != nullptr );
		REQUIRE( reader->get_descriptor() == image_descriptor(
			make_span(extents), 2, numerical_type::uint8) );

		auto destination =
			make_host_array<std::uint8_t>(extents, numerical_type::uint8);
		reader->read(array_ref(destination), whole_of(extents));

		REQUIRE( values_of<std::uint8_t>(destination, extents) ==
			counting<std::uint8_t>(12) );
	}

	SECTION( "a file of several pages opens as a stack" )
	{
		write_striped_file<std::int16_t>(
			path.get(), "w", 4, 3, SAMPLEFORMAT_INT, COMPRESSION_NONE, 3,
			{counting<std::int16_t>(12), counting<std::int16_t>(12)}
		);

		const auto reader = format.open(image_probe(path.get()));

		const std::vector<std::size_t> extents = {2, 3, 4};

		REQUIRE( reader->get_descriptor() == image_descriptor(
			make_span(extents), 2, numerical_type::int16) );
	}

	SECTION( "a file this format can not transfer does not open" )
	{
		write_rgb_file(path.get(), 4, 3);

		const image_probe probe(path.get());

		REQUIRE( format.get_suitability(probe) == backend_priority::normal );
		REQUIRE_THROWS_AS( format.open(probe), image_format_error );
	}
}
