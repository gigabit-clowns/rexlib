// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/tests/assets.hpp>

#include <xmipp4/core/system/dynamic_library.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <sstream>
#include <string>

using namespace rex;

TEST_CASE( "open a dynamic library", "[dynamic_library]" ) 
{
	dynamic_library dl(get_mock_plugin_path("dummy-plugin"));

	REQUIRE( dl.is_open() );
	REQUIRE( dl.get_symbol("lorem_ipsum") == nullptr );

	const auto *symbol = dl.get_symbol("xmipp4_get_plugin");
	REQUIRE( symbol != nullptr );
}

TEST_CASE( "default construct dynamic_library", "[dynamic_library]" ) 
{
	dynamic_library lib;
	REQUIRE( lib.is_open() == false );
}

TEST_CASE( "recover shared library path from symbol", "[dynamic_library]" ) 
{
	// Open a dynamic library and query a symbol.
	const auto path = get_mock_plugin_path("dummy-plugin");
	dynamic_library dl(path);
	const auto *symbol = dl.get_symbol("xmipp4_get_plugin");

	// Recover path from loaded symbol
	auto recovered = dynamic_library::query_symbol_filename(symbol);

	// query_symbol_filename returns the native absolute path of the module,
	// so only its tail relative to the asset root is meaningful to compare.
	// Normalize separators as it uses the platform convention (backslashes on
	// Windows) whereas the asset root is baked in with forward slashes.
	auto relative = path.substr(get_asset_root().size());
	std::replace(relative.begin(), relative.end(), '\\', '/');
	std::replace(recovered.begin(), recovered.end(), '\\', '/');

	// The recovered path should end with the asset-relative path even though
	// it is absolute.
	REQUIRE( recovered.size() >= relative.size() );
	CHECK(
		recovered.compare(
			recovered.size() - relative.size(), relative.size(), relative
		) == 0
	);
}
