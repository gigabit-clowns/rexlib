// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/library_version.hpp>
#include <rexlib/core/version_macros.h>

using namespace rexlib;

TEST_CASE(
	"get_library_version should return the version of the headers",
	"[library_version]"
)
{
	REQUIRE(
		get_library_version() == version(
			REXLIB_VERSION_MAJOR,
			REXLIB_VERSION_MINOR,
			REXLIB_VERSION_PATCH
		)
	);
}
