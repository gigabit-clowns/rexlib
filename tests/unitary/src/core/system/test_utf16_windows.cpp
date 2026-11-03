// SPDX-License-Identifier: GPL-3.0-only

#include <core/system/utf16_windows.hpp>

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

using namespace rexlib;

TEST_CASE( "utf8_to_utf16 should decode UTF-8", "[utf16_windows]" )
{
	CHECK( utf8_to_utf16("plugins-\xc3\xb1") == L"plugins-ñ" );
	CHECK( utf8_to_utf16("\xf0\x9f\x94\xac") == L"\U0001F52C" );
	CHECK( utf8_to_utf16("").empty() );
}

TEST_CASE(
	"utf8_to_utf16 should reject what is not UTF-8",
	"[utf16_windows]"
)
{
	REQUIRE_THROWS_AS( utf8_to_utf16("\xff"), std::invalid_argument );
}

TEST_CASE( "utf16_to_utf8 should encode UTF-8", "[utf16_windows]" )
{
	CHECK( utf16_to_utf8(L"plugins-ñ") == "plugins-\xc3\xb1" );
	CHECK( utf16_to_utf8(L"\U0001F52C") == "\xf0\x9f\x94\xac" );
	CHECK( utf16_to_utf8(L"").empty() );
}

TEST_CASE(
	"utf16_to_utf8 should replace a lone surrogate",
	"[utf16_windows]"
)
{
	const std::wstring lone_surrogate(1, static_cast<wchar_t>(0xD800));

	CHECK( utf16_to_utf8(lone_surrogate) == "\xef\xbf\xbd" );
}
