// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/mrc/mrc_extensions.hpp>

#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::mrc;

TEST_CASE( "the extensions an MRC file is read from", "[mrc_extensions]" )
{
	SECTION( "the names the format is known by are read" )
	{
		REQUIRE( is_readable_extension(".mrc") );
		REQUIRE( is_readable_extension(".mrcs") );
		REQUIRE( is_readable_extension(".map") );
	}

	SECTION( "the names IMOD gives its own files are read too" )
	{
		REQUIRE( is_readable_extension(".st") );
		REQUIRE( is_readable_extension(".rec") );
		REQUIRE( is_readable_extension(".ali") );
	}

	SECTION( "anything else is not" )
	{
		REQUIRE_FALSE( is_readable_extension(".tif") );
		REQUIRE_FALSE( is_readable_extension(".h5") );
		REQUIRE_FALSE( is_readable_extension(".em") );
		REQUIRE_FALSE( is_readable_extension(".eer") );
		REQUIRE_FALSE( is_readable_extension("") );
		REQUIRE_FALSE( is_readable_extension(".") );
	}

	SECTION( "the leading dot is part of it" )
	{
		// image_probe reports the extension with its dot, so a bare suffix
		// is not one of these.
		REQUIRE_FALSE( is_readable_extension("mrc") );
	}

	SECTION( "it is matched as given, already folded to lower case" )
	{
		// image_probe folds the case before this is reached, so an upper
		// case extension never arrives here and is not matched.
		REQUIRE_FALSE( is_readable_extension(".MRC") );
	}
}

TEST_CASE( "the extensions an MRC file is created with", "[mrc_extensions]" )
{
	SECTION( "the names the format is known by are created" )
	{
		REQUIRE( is_writable_extension(".mrc") );
		REQUIRE( is_writable_extension(".mrcs") );
		REQUIRE( is_writable_extension(".map") );
	}

	SECTION( "the names IMOD gives its own files are not" )
	{
		// Reading one of these is recognizing a file somebody else named.
		// Creating one would be naming a file something this format has no
		// business claiming.
		REQUIRE_FALSE( is_writable_extension(".st") );
		REQUIRE_FALSE( is_writable_extension(".rec") );
		REQUIRE_FALSE( is_writable_extension(".ali") );
	}

	SECTION( "anything else is not" )
	{
		REQUIRE_FALSE( is_writable_extension(".tif") );
		REQUIRE_FALSE( is_writable_extension("") );
	}
}

TEST_CASE( "every extension written is one that can be read back",
	"[mrc_extensions]" )
{
	// The asymmetry only goes one way: a file this format creates must be a
	// file it recognizes afterwards.
	const std::vector<std::string> written = {
		".mrc", ".mrcs", ".map", ".st", ".rec", ".ali", ".tif", ".em", ""
	};

	for (const auto &extension : written)
	{
		if (is_writable_extension(extension))
		{
			REQUIRE( is_readable_extension(extension) );
		}
	}
}

TEST_CASE(
	"a single section is read as a stack only from a file named as one",
	"[mrc_extensions]"
)
{
	SECTION( ".mrcs names a stack, as RELION uses it" )
	{
		REQUIRE( get_single_section(".mrcs") ==
			mrc_single_section::image_stack );
	}

	SECTION( "any other extension leaves a single image" )
	{
		REQUIRE( get_single_section(".mrc") == mrc_single_section::image );
		REQUIRE( get_single_section(".map") == mrc_single_section::image );
		REQUIRE( get_single_section(".st") == mrc_single_section::image );
		REQUIRE( get_single_section("") == mrc_single_section::image );
	}
}
