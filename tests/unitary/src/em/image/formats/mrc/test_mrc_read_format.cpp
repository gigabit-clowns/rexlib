// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <em/image/formats/mrc/mrc_read_format.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include "../../fixtures/scoped_path.hpp"
#include "fixtures/mrc_test_file.hpp"

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::mrc;
using namespace rexlib::test;

TEST_CASE( "the MRC format claims the files it can read",
	"[mrc_read_format]" )
{
	const scoped_path path("reader_claimed.mrc");
	const mrc_read_format format;

	SECTION( "it is named" )
	{
		REQUIRE( format.get_name() == "MRC" );
	}

	SECTION( "a file carrying the identifier is claimed" )
	{
		write_file(path.get(), make_file(4, 3, 1, 0, 2, counting(12)));

		REQUIRE( format.get_suitability(image_probe(path.get())) ==
			backend_priority::normal );
	}

	SECTION( "the identifier is claimed whatever the extension" )
	{
		const std::string other = path.get() + ".unknown";
		write_file(other, make_file(4, 3, 1, 0, 2, counting(12)));

		REQUIRE( format.get_suitability(image_probe(other)) ==
			backend_priority::normal );

		std::remove(other.c_str());
	}

	SECTION( "a long enough file of a known extension is claimed weakly" )
	{
		auto raw = make_file(4, 3, 1, 0, 2, counting(12));
		std::memcpy(raw.data() + 208, "\0\0\0\0", 4);
		write_file(path.get(), raw);

		REQUIRE( format.get_suitability(image_probe(path.get())) ==
			backend_priority::fallback );
	}

	SECTION( "a file too short to hold a header is not claimed" )
	{
		write_file(path.get(), std::vector<char>(16, '\0'));

		REQUIRE( format.get_suitability(image_probe(path.get())) ==
			backend_priority::unsupported );
	}

	SECTION( "a file that is not there is not claimed" )
	{
		REQUIRE( format.get_suitability(image_probe(path.get())) ==
			backend_priority::unsupported );
	}

	SECTION( "a claimed file opens into a reader" )
	{
		write_file(path.get(), make_file(4, 3, 1, 0, 2, counting(12)));

		const auto reader = format.open(image_probe(path.get()));

		REQUIRE( reader != nullptr );
		REQUIRE( reader->get_descriptor().get_core_rank() == 2 );
	}
}

TEST_CASE( "the MRC read format opens a file as its header states it",
	"[mrc_read_format]" )
{
	const scoped_path path("read_format_opened.mrc");
	const mrc_read_format format;

	SECTION( "a stack of images reports its shape and data type" )
	{
		write_file(path.get(), make_file(4, 3, 2, 0, 2, counting(24)));

		const auto reader = format.open(image_probe(path.get()));

		const std::vector<std::size_t> extents = {2, 3, 4};
		const image_descriptor expected(
			make_span(extents),
			2,
			numerical_type::float32
		);

		REQUIRE( reader->get_descriptor() == expected );
	}

	SECTION( "the values that follow the header are read" )
	{
		const auto values = counting(24);
		write_file(path.get(), make_file(4, 3, 2, 0, 2, values));

		const auto reader = format.open(image_probe(path.get()));

		const std::vector<std::size_t> extents = {2, 3, 4};
		auto storage = std::make_shared<host_buffer>(
			values.size() * sizeof(float),
			alignof(float)
		);
		array destination(
			storage,
			array_descriptor(
				strided_layout::make_contiguous_layout(make_span(extents)),
				numerical_type::float32
			)
		);

		image_transfer_plan regions(image_transfer_shape(extents, 3, 3));
		const std::vector<std::size_t> origin(3, 0);
		regions.add(make_span(origin), make_span(origin));

		reader->read(array_ref(destination), regions);

		const auto *data = static_cast<const float*>(storage->get_host_ptr());

		REQUIRE( std::vector<float>(data, data + values.size()) == values );
	}
}

TEST_CASE( "the MRC read format refuses a file that contradicts its header",
	"[mrc_read_format]" )
{
	const scoped_path path("read_format_bad_header.mrc");
	const mrc_read_format format;

	SECTION( "one without the identifier" )
	{
		auto raw = make_file(4, 3, 1, 0, 2, counting(12));
		raw[208] = 'X';
		write_file(path.get(), raw);

		REQUIRE_THROWS_AS(
			format.open(image_probe(path.get())),
			image_format_error
		);
	}

	SECTION( "one shorter than the shape it states, naming it" )
	{
		auto raw = make_file(4, 3, 2, 0, 2, counting(24));
		raw.resize(raw.size() - sizeof(float));
		write_file(path.get(), raw);

		REQUIRE_THROWS_MATCHES(
			format.open(image_probe(path.get())),
			image_format_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith(path.get() + ": ")
			)
		);
	}
}

TEST_CASE(
	"the MRC read format reads a single section as its file is named",
	"[mrc_read_format]"
)
{
	const mrc_read_format format;

	SECTION( "a stack of one image from a .mrcs file" )
	{
		const scoped_path path("read_format_single.mrcs");
		write_file(path.get(), make_file(4, 3, 1, 0, 2, counting(12)));

		const auto reader = format.open(image_probe(path.get()));
		const auto extents = reader->get_descriptor().get_extents();

		REQUIRE( std::vector<std::size_t>(extents.begin(), extents.end()) ==
			std::vector<std::size_t>{1, 3, 4} );
	}

	SECTION( "a single image from any other" )
	{
		const scoped_path path("read_format_single.mrc");
		write_file(path.get(), make_file(4, 3, 1, 0, 2, counting(12)));

		const auto reader = format.open(image_probe(path.get()));
		const auto extents = reader->get_descriptor().get_extents();

		REQUIRE( std::vector<std::size_t>(extents.begin(), extents.end()) ==
			std::vector<std::size_t>{3, 4} );
	}
}
