// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/operating_system.h>
#include <rexlib/core/platform/compiler.h>

#include <string>

#ifndef REXLIB_TEST_ASSET_ROOT
	#error "REXLIB_TEST_ASSET_ROOT is not defined. Link against " \
		"rexlib-test-assets-interface to consume the shared test assets."
#endif

namespace rexlib
{

inline std::string get_asset_root()
{
	return REXLIB_TEST_ASSET_ROOT;
}

inline std::string get_path_separator()
{
	#if REXLIB_WINDOWS
		return "\\";
	#elif REXLIB_APPLE || REXLIB_LINUX
		return "/";
	#else
		#error "Unknown platform"
	#endif
}

inline std::string get_text_file_path()
{
	return get_asset_root() + get_path_separator() + "lorem_ipsum.txt";
}

inline std::string get_test_plugin_directory()
{
	return get_asset_root() + get_path_separator() + "plugins";
}

// Holds a copy of the dummy plugin, under the name plugins-ñ written in UTF-8.
inline std::string get_non_ascii_test_plugin_directory()
{
	return get_asset_root() + get_path_separator() + "plugins-\xc3\xb1";
}

inline std::string get_mock_plugin_file_name(const std::string &name)
{
	#if REXLIB_WINDOWS && !REXLIB_MINGW
		return name + ".dll";
	#elif REXLIB_WINDOWS
		return "lib" + name + ".dll";
	#elif REXLIB_APPLE || REXLIB_LINUX
		return "lib" + name + ".so";
	#else
		#error "Unknown platform"
	#endif
}

inline std::string get_mock_plugin_path(const std::string &name)
{
	return get_test_plugin_directory() + get_path_separator() +
		get_mock_plugin_file_name(name);
}

} // namespace rexlib
