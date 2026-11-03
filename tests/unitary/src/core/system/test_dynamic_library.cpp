// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/tests/assets.hpp>

#include <rexlib/core/system/dynamic_library.hpp>
#include <rexlib/core/exceptions/dynamic_library_error.hpp>
#include <rexlib/core/platform/operating_system.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <algorithm>
#include <sstream>
#include <string>

using namespace rexlib;

TEST_CASE( "open a dynamic library", "[dynamic_library]" )
{
	dynamic_library dl(get_mock_plugin_path("dummy-plugin"));

	REQUIRE( dl.is_open() );
	REQUIRE( dl.get_symbol("lorem_ipsum") == nullptr );

	const auto *symbol = dl.get_symbol("rexlib_get_plugin");
	REQUIRE( symbol != nullptr );
}

TEST_CASE( "default construct dynamic_library", "[dynamic_library]" )
{
	dynamic_library lib;
	REQUIRE( lib.is_open() == false );
	REQUIRE( lib.get_symbol("rexlib_get_plugin") == nullptr );
}

// Whether path ends with the part of expected below the asset root. A path
// recovered from a symbol is absolute and uses the separators of the
// platform, which may differ from those of the asset root.
static bool ends_with_asset_path(std::string path, const std::string &expected)
{
	auto relative = expected.substr(get_asset_root().size());
	std::replace(relative.begin(), relative.end(), '\\', '/');
	std::replace(path.begin(), path.end(), '\\', '/');
	return path.size() >= relative.size() &&
		path.compare(
			path.size() - relative.size(), relative.size(), relative
		) == 0;
}

TEST_CASE( "recover shared library path from symbol", "[dynamic_library]" )
{
	const auto path = get_mock_plugin_path("dummy-plugin");
	dynamic_library dl(path);
	const auto *symbol = dl.get_symbol("rexlib_get_plugin");

	const auto recovered = dynamic_library::query_symbol_filename(symbol);

	CHECK( ends_with_asset_path(recovered, path) );
}

TEST_CASE(
	"open a dynamic library whose path is not ASCII",
	"[dynamic_library]"
)
{
	const auto path = get_non_ascii_test_plugin_directory() +
		get_path_separator() + get_mock_plugin_file_name("dummy-plugin");
	dynamic_library dl(path);
	REQUIRE( dl.is_open() );

	const auto *symbol = dl.get_symbol("rexlib_get_plugin");
	REQUIRE( symbol != nullptr );

	const auto recovered = dynamic_library::query_symbol_filename(symbol);
	CHECK( ends_with_asset_path(recovered, path) );
}

TEST_CASE(
	"opening a missing dynamic library should throw",
	"[dynamic_library]"
)
{
	const auto path = get_asset_root() + get_path_separator() + "nowhere";

	REQUIRE_THROWS_AS( dynamic_library(path), dynamic_library_error );
	REQUIRE_THROWS_WITH(
		dynamic_library(path),
		Catch::Matchers::ContainsSubstring("nowhere")
	);
}

#if REXLIB_POSIX
TEST_CASE(
	"opening a dynamic library that misses a symbol should throw",
	"[dynamic_library]"
)
{
	REQUIRE_THROWS_AS(
		dynamic_library(get_mock_plugin_path("plugin-with-missing-symbol")),
		dynamic_library_error
	);
}
#endif
