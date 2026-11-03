// SPDX-License-Identifier: GPL-3.0-only

#include <core/plugin_compatibility_check.hpp>

#include <rexlib/core/exceptions/plugin_load_error.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstdint>
#include <string>

using namespace rexlib;

using Catch::Matchers::ContainsSubstring;

static plugin_compatibility make_compatibility(
	std::uint32_t major,
	std::uint32_t minor,
	std::uint32_t patch
)
{
	plugin_compatibility compatibility = REXLIB_PLUGIN_COMPATIBILITY;
	compatibility.rexlib_version_major = major;
	compatibility.rexlib_version_minor = minor;
	compatibility.rexlib_version_patch = patch;
	return compatibility;
}

TEST_CASE(
	"check_plugin_compatibility should accept a plugin built against the "
	"library",
	"[plugin_compatibility_check]"
)
{
	const plugin_compatibility compatibility = REXLIB_PLUGIN_COMPATIBILITY;

	REQUIRE_NOTHROW(
		check_plugin_compatibility(compatibility, compatibility)
	);
}

TEST_CASE(
	"check_plugin_compatibility should accept another patch version",
	"[plugin_compatibility_check]"
)
{
	const auto major = GENERATE(0u, 1u);

	REQUIRE_NOTHROW(
		check_plugin_compatibility(
			make_compatibility(major, 3, 1),
			make_compatibility(major, 3, 0)
		)
	);
}

TEST_CASE(
	"check_plugin_compatibility should refuse another minor version before "
	"1.0",
	"[plugin_compatibility_check]"
)
{
	const auto plugin_minor = GENERATE(2u, 4u);
	const auto plugin = make_compatibility(0, plugin_minor, 0);
	const auto library = make_compatibility(0, 3, 0);

	REQUIRE_THROWS_AS(
		check_plugin_compatibility(plugin, library),
		plugin_load_error
	);
	REQUIRE_THROWS_WITH(
		check_plugin_compatibility(plugin, library),
		ContainsSubstring("rexlib 0." + std::to_string(plugin_minor)) &&
		ContainsSubstring("rexlib 0.3.0 is running") &&
		ContainsSubstring("before 1.0")
	);
}

TEST_CASE(
	"check_plugin_compatibility should accept an older minor version from "
	"1.0",
	"[plugin_compatibility_check]"
)
{
	REQUIRE_NOTHROW(
		check_plugin_compatibility(
			make_compatibility(1, 2, 7),
			make_compatibility(1, 3, 0)
		)
	);
}

TEST_CASE(
	"check_plugin_compatibility should refuse a newer minor version from 1.0",
	"[plugin_compatibility_check]"
)
{
	const auto plugin = make_compatibility(1, 4, 0);
	const auto library = make_compatibility(1, 3, 0);

	REQUIRE_THROWS_WITH(
		check_plugin_compatibility(plugin, library),
		ContainsSubstring("rexlib 1.4.0") &&
		ContainsSubstring("rexlib 1.3.0 is running") &&
		ContainsSubstring("no newer a minor version")
	);
}

TEST_CASE(
	"check_plugin_compatibility should refuse another major version",
	"[plugin_compatibility_check]"
)
{
	const auto plugin_major = GENERATE(0u, 2u);

	REQUIRE_THROWS_AS(
		check_plugin_compatibility(
			make_compatibility(plugin_major, 0, 0),
			make_compatibility(1, 0, 0)
		),
		plugin_load_error
	);
}

TEST_CASE(
	"check_plugin_compatibility should refuse another C++ ABI",
	"[plugin_compatibility_check]"
)
{
	const plugin_compatibility library = {
		sizeof(plugin_compatibility), 0, 1, 0,
		REXLIB_CPP_ABI_ITANIUM, REXLIB_CPP_LIBSTDCXX, 1, 0
	};

	SECTION( "another family" )
	{
		const plugin_compatibility plugin = {
			sizeof(plugin_compatibility), 0, 1, 0,
			REXLIB_CPP_ABI_MSVC, REXLIB_CPP_MSVC_STL, 0, 1
		};
		REQUIRE_THROWS_WITH(
			check_plugin_compatibility(plugin, library),
			ContainsSubstring(
				"built for the MSVC C++ ABI with the MSVC standard library "
				"(_ITERATOR_DEBUG_LEVEL=0) and the debug runtime"
			) &&
			ContainsSubstring(
				"rexlib was built for the Itanium C++ ABI with libstdc++ "
				"(_GLIBCXX_USE_CXX11_ABI=1)"
			)
		);
	}

	SECTION( "another standard library" )
	{
		const plugin_compatibility plugin = {
			sizeof(plugin_compatibility), 0, 1, 0,
			REXLIB_CPP_ABI_ITANIUM, REXLIB_CPP_LIBCXX, 1, 0
		};
		REQUIRE_THROWS_WITH(
			check_plugin_compatibility(plugin, library),
			ContainsSubstring("libc++ (_LIBCPP_ABI_VERSION=1)")
		);
	}

	SECTION( "another ABI of the standard library" )
	{
		const plugin_compatibility plugin = {
			sizeof(plugin_compatibility), 0, 1, 0,
			REXLIB_CPP_ABI_ITANIUM, REXLIB_CPP_LIBSTDCXX, 0, 0
		};
		REQUIRE_THROWS_WITH(
			check_plugin_compatibility(plugin, library),
			ContainsSubstring("libstdc++ (_GLIBCXX_USE_CXX11_ABI=0)")
		);
	}
}

TEST_CASE(
	"check_plugin_compatibility should refuse another runtime of MSVC",
	"[plugin_compatibility_check]"
)
{
	const plugin_compatibility plugin = {
		sizeof(plugin_compatibility), 0, 1, 0,
		REXLIB_CPP_ABI_MSVC, REXLIB_CPP_MSVC_STL, 2, 1
	};
	const plugin_compatibility library = {
		sizeof(plugin_compatibility), 0, 1, 0,
		REXLIB_CPP_ABI_MSVC, REXLIB_CPP_MSVC_STL, 2, 0
	};

	REQUIRE_THROWS_WITH(
		check_plugin_compatibility(plugin, library),
		ContainsSubstring("(_ITERATOR_DEBUG_LEVEL=2) and the debug runtime") &&
		ContainsSubstring("(_ITERATOR_DEBUG_LEVEL=2) and the release runtime")
	);
}

TEST_CASE(
	"check_plugin_compatibility should refuse a record shorter than the "
	"first",
	"[plugin_compatibility_check]"
)
{
	plugin_compatibility plugin = REXLIB_PLUGIN_COMPATIBILITY;
	plugin.size = 4;
	const plugin_compatibility library = REXLIB_PLUGIN_COMPATIBILITY;

	REQUIRE_THROWS_WITH(
		check_plugin_compatibility(plugin, library),
		ContainsSubstring("holds 4 bytes")
	);
}
