// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/mrc/mrc_read_format.hpp>

#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_reader.hpp>

#include "../../fixtures/scoped_path.hpp"
#include "fixtures/mrc_test_file.hpp"

#include <cstdio>
#include <cstring>
#include <string>
#include <cstddef>
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

		const auto extents =
			format.open(image_probe(path.get()))->get_descriptor()
				.get_extents();

		REQUIRE( std::vector<std::size_t>(extents.begin(), extents.end()) ==
			std::vector<std::size_t>{1, 3, 4} );
	}

	SECTION( "a single image from any other" )
	{
		const scoped_path path("read_format_single.mrc");
		write_file(path.get(), make_file(4, 3, 1, 0, 2, counting(12)));

		const auto extents =
			format.open(image_probe(path.get()))->get_descriptor()
				.get_extents();

		REQUIRE( std::vector<std::size_t>(extents.begin(), extents.end()) ==
			std::vector<std::size_t>{3, 4} );
	}
}
