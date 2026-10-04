// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <em/image/formats/mrc/mrc_write_format.hpp>

#include <em/image/formats/mrc/mrc_constants.hpp>
#include <em/image/formats/mrc/mrc_header.hpp>

#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_writer.hpp>

#include "../../fixtures/scoped_path.hpp"

#include <boost/filesystem/operations.hpp>

#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::mrc;

namespace
{

image_descriptor make_descriptor(
	const std::vector<std::size_t> &extents,
	std::size_t core_rank,
	numerical_type data_type = numerical_type::float32
)
{
	return image_descriptor(make_span(extents), core_rank, data_type);
}

mrc_header header_of(const std::string &path)
{
	std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);
	std::vector<rexlib::byte> raw(header_size);
	input.read(reinterpret_cast<char*>(raw.data()), header_size);

	return parse_header(make_span(raw.data(), raw.size()));
}

} // anonymous namespace

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

TEST_CASE(
	"the MRC write format creates a stack of one only where it reads back",
	"[mrc_write_format]"
)
{
	const mrc_write_format format;
	const std::vector<std::size_t> extents = {1, 3, 4};
	const image_descriptor descriptor(
		make_span(extents),
		2,
		numerical_type::float32
	);

	SECTION( "in a .mrcs file" )
	{
		const scoped_path path("write_format_single.mrcs");

		const auto writer = format.open(
			image_probe(path.get()),
			descriptor,
			image_metadata()
		);

		REQUIRE( writer->get_descriptor() == descriptor );
	}

	SECTION( "not in any other, which would read it back as an image" )
	{
		const scoped_path path("write_format_single.mrc");

		REQUIRE_THROWS_MATCHES(
			format.open(image_probe(path.get()), descriptor, image_metadata()),
			unsupported_operation_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith(path.get() + ": ")
			)
		);
		REQUIRE_FALSE( boost::filesystem::exists(path.get()) );
	}

	SECTION( "nor a single image in a .mrcs file, which reads as a stack" )
	{
		const scoped_path path("write_format_image.mrcs");
		const std::vector<std::size_t> image = {3, 4};

		REQUIRE_THROWS_AS(
			format.open(
				image_probe(path.get()),
				make_descriptor(image, 2),
				image_metadata()
			),
			unsupported_operation_error
		);
	}
}

TEST_CASE( "the MRC write format creates a file with its header written",
	"[mrc_write_format]" )
{
	const scoped_path path("write_format_created.mrc");
	const mrc_write_format format;
	const std::vector<std::size_t> extents = {2, 3, 4};

	{
		const auto writer = format.open(
			image_probe(path.get()),
			make_descriptor(extents, 2),
			image_metadata()
		);
	}

	SECTION( "the file is laid out in full before anything is written" )
	{
		REQUIRE( boost::filesystem::file_size(path.get()) ==
			1024 + 24 * sizeof(float) );
	}

	SECTION( "the header states the shape the file was created with" )
	{
		const auto header = header_of(path.get());

		REQUIRE( header.get_column_count() == 4 );
		REQUIRE( header.get_row_count() == 3 );
		REQUIRE( header.get_section_count() == 2 );
		REQUIRE( header.get_space_group() == 0 );
	}

	SECTION( "the file records the library that wrote it" )
	{
		// The header owns the labels the span refers to, so it has to
		// outlive it.
		const auto header = header_of(path.get());
		const auto labels = header.get_labels();

		REQUIRE( labels.size() == 1 );
		REQUIRE( labels[0].compare(0, 17, "Created by rexlib") == 0 );
	}
}

TEST_CASE( "the MRC write format refuses what the format can not hold",
	"[mrc_write_format]" )
{
	const scoped_path path("write_format_refused.mrc");
	const mrc_write_format format;

	SECTION( "a shape it has no file for" )
	{
		const std::vector<std::size_t> line = {4};

		REQUIRE_THROWS_AS(
			format.open(
				image_probe(path.get()),
				make_descriptor(line, 1),
				image_metadata()
			),
			unsupported_operation_error
		);
	}

	SECTION( "a data type no mode holds" )
	{
		const std::vector<std::size_t> extents = {2, 3};

		REQUIRE_THROWS_AS(
			format.open(
				image_probe(path.get()),
				make_descriptor(extents, 2, numerical_type::float64),
				image_metadata()
			),
			unsupported_operation_error
		);
	}
}
