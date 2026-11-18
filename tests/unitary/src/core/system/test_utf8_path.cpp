// SPDX-License-Identifier: GPL-3.0-only

#include <core/system/utf8_path.hpp>

#include <rexlib/core/platform/operating_system.h>

#include <catch2/catch_test_macros.hpp>

#include <string>

using namespace rexlib;

TEST_CASE(
	"path_to_utf8 should spell what utf8_to_path was given",
	"[utf8_path]"
)
{
	const std::string utf8 = "plugins-\xc3\xb1/\xe6\x97\xa5\xe6\x9c\xac";

	CHECK( path_to_utf8(utf8_to_path(utf8)) == utf8 );
}

TEST_CASE(
	"utf8_to_path should read an ASCII path as it is",
	"[utf8_path]"
)
{
	CHECK( utf8_to_path("rexlib-plugins").string() == "rexlib-plugins" );
}

#if REXLIB_WINDOWS
TEST_CASE(
	"utf8_to_path should decode UTF-8 whatever the code page",
	"[utf8_path]"
)
{
	CHECK( utf8_to_path("plugins-\xc3\xb1").wstring() == L"plugins-ñ" );
}
#endif
