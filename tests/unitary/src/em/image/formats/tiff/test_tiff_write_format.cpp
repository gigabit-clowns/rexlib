// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <em/image/formats/tiff/tiff_write_format.hpp>

#include <em/image/formats/tiff/tiff_reader.hpp>

#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/exceptions/image_file_error.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_writer.hpp>

#include "../../fixtures/scoped_path.hpp"
#include "fixtures/host_array.hpp"
#include "fixtures/tiff_test_file.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::tiff;
using namespace rexlib::test;

namespace
{

image_descriptor describe(
	const std::vector<std::size_t> &extents,
	std::size_t core_rank,
	numerical_type data_type
)
{
	return image_descriptor(make_span(extents), core_rank, data_type);
}

} // anonymous namespace

TEST_CASE( "the TIFF format claims the files it can create",
	"[tiff_write_format]" )
{
	const tiff_write_format format;

	SECTION( "it is named" )
	{
		REQUIRE( format.get_name() == "TIFF" );
	}

	SECTION( "a file of either extension is claimed, existing or not" )
	{
		REQUIRE( format.get_suitability(image_probe("absent.tif")) ==
			backend_priority::normal );
		REQUIRE( format.get_suitability(image_probe("absent.tiff")) ==
			backend_priority::normal );
		REQUIRE( format.get_suitability(image_probe("absent.TIF")) ==
			backend_priority::normal );
	}

	SECTION( "a file of another extension is not" )
	{
		REQUIRE( format.get_suitability(image_probe("absent.mrc")) ==
			backend_priority::unsupported );
		REQUIRE( format.get_suitability(image_probe("absent")) ==
			backend_priority::unsupported );
	}

	SECTION( "what an existing file holds does not matter" )
	{
		const scoped_path path("tiff_write_format_claimed.tif");
		write_file(path.get(), std::vector<char>(64, 'x'));

		REQUIRE( format.get_suitability(image_probe(path.get())) ==
			backend_priority::normal );
	}
}

TEST_CASE( "the TIFF write format creates the file it is asked for",
	"[tiff_write_format]" )
{
	const scoped_path path("tiff_write_format_created.tif");
	const tiff_write_format format;

	SECTION( "an image" )
	{
		const std::vector<std::size_t> extents = {4, 5};
		const auto descriptor = describe(extents, 2, numerical_type::uint8);
		const auto values = counting<std::uint8_t>(20);
		const auto source = make_host_array<std::uint8_t>(
			extents, numerical_type::uint8, values);

		{
			const auto writer = format.open(
				image_probe(path.get()), descriptor, image_metadata());

			REQUIRE( writer != nullptr );
			REQUIRE( writer->get_descriptor() == descriptor );

			writer->write(const_array_ref(source), whole_of(extents));
		}

		const tiff_reader reader(path.get());
		auto destination =
			make_host_array<std::uint8_t>(extents, numerical_type::uint8);
		reader.read(array_ref(destination), whole_of(extents));

		REQUIRE( reader.get_descriptor() == descriptor );
		REQUIRE( values_of<std::uint8_t>(destination, extents) == values );
	}

	SECTION( "a stack of images" )
	{
		const std::vector<std::size_t> extents = {3, 4, 5};
		const auto descriptor = describe(extents, 2, numerical_type::float32);
		const auto values = counting<float>(60, 0.5F);
		const auto source = make_host_array<float>(
			extents, numerical_type::float32, values);

		{
			const auto writer = format.open(
				image_probe(path.get()), descriptor, image_metadata());
			writer->write(const_array_ref(source), whole_of(extents));
		}

		const tiff_reader reader(path.get());

		REQUIRE( reader.get_descriptor() == descriptor );
	}

	SECTION( "in place of the file that was there" )
	{
		write_file(path.get(), std::vector<char>(64, 'x'));

		const std::vector<std::size_t> extents = {4, 5};
		const auto descriptor = describe(extents, 2, numerical_type::int16);
		const auto source = make_host_array<std::int16_t>(
			extents, numerical_type::int16);

		{
			const auto writer = format.open(
				image_probe(path.get()), descriptor, image_metadata());
			writer->write(const_array_ref(source), whole_of(extents));
		}

		REQUIRE( tiff_reader(path.get()).get_descriptor() == descriptor );
	}
}

TEST_CASE( "the TIFF write format refuses what a file can not hold",
	"[tiff_write_format]" )
{
	const scoped_path path("tiff_write_format_refused.tif");
	const tiff_write_format format;
	const image_probe probe(path.get());

	SECTION( "a stack of one image, which would read back as an image" )
	{
		const std::vector<std::size_t> extents = {1, 4, 5};

		REQUIRE_THROWS_MATCHES(
			format.open(
				probe,
				describe(extents, 2, numerical_type::uint8),
				image_metadata()
			),
			unsupported_operation_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith(path.get()) &&
				Catch::Matchers::ContainsSubstring("stack of one")
			)
		);
	}

	SECTION( "a volume" )
	{
		const std::vector<std::size_t> extents = {3, 4, 5};

		REQUIRE_THROWS_AS(
			format.open(
				probe,
				describe(extents, 3, numerical_type::uint8),
				image_metadata()
			),
			unsupported_operation_error
		);
	}

	SECTION( "a single row" )
	{
		const std::vector<std::size_t> extents = {5};

		REQUIRE_THROWS_AS(
			format.open(
				probe,
				describe(extents, 1, numerical_type::uint8),
				image_metadata()
			),
			unsupported_operation_error
		);
	}

	SECTION( "a stack of stacks" )
	{
		const std::vector<std::size_t> extents = {2, 3, 4, 5};

		REQUIRE_THROWS_AS(
			format.open(
				probe,
				describe(extents, 2, numerical_type::uint8),
				image_metadata()
			),
			unsupported_operation_error
		);
	}

	SECTION( "an extent of zero" )
	{
		const std::vector<std::size_t> extents = {0, 5};

		REQUIRE_THROWS_AS(
			format.open(
				probe,
				describe(extents, 2, numerical_type::uint8),
				image_metadata()
			),
			unsupported_operation_error
		);
	}

	SECTION( "a data type with no sample" )
	{
		const std::vector<std::size_t> extents = {4, 5};

		REQUIRE_THROWS_AS(
			format.open(
				probe,
				describe(extents, 2, numerical_type::complex_float32),
				image_metadata()
			),
			unsupported_operation_error
		);
	}

	SECTION( "a file that can not be created" )
	{
		const std::vector<std::size_t> extents = {4, 5};
		const auto nowhere =
			get_scratch_path("tiff_write_format_no_such_directory") +
			"/a.tif";

		REQUIRE_THROWS_AS(
			format.open(
				image_probe(nowhere),
				describe(extents, 2, numerical_type::uint8),
				image_metadata()
			),
			image_file_error
		);
	}
}
