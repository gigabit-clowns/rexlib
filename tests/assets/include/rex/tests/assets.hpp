// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/platform/operating_system.h>
#include <rex/core/platform/compiler.h>

#include <string>

#ifndef REXLIB_TEST_ASSET_ROOT
	#error "REXLIB_TEST_ASSET_ROOT is not defined. Link against " \
	       "rex-test-assets-interface to consume the shared test assets."
#endif

namespace rexlib
{

inline std::string get_asset_root()
{
	return REXLIB_TEST_ASSET_ROOT;
}

inline std::string get_text_file_path()
{
	#if REXLIB_WINDOWS
		return get_asset_root() + "\\" + "lorem_ipsum.txt";
	#elif REXLIB_APPLE || REXLIB_LINUX
		return get_asset_root() + "/" + "lorem_ipsum.txt";
	#else
		#error "Unknown platform"
	#endif
}

inline std::string get_test_plugin_directory()
{
	#if REXLIB_WINDOWS
		return get_asset_root() + "\\plugins";
	#elif REXLIB_APPLE || REXLIB_LINUX
		return get_asset_root() + "/plugins";
	#else
		#error "Unknown platform"
	#endif
}

inline std::string get_mock_plugin_path(const std::string &name)
{

	#if REXLIB_WINDOWS
		#if REXLIB_MINGW
			return get_asset_root() + "\\plugins\\lib" + name + ".dll";
		#else
			return get_asset_root() + "\\plugins\\" + name + ".dll";
		#endif
	#elif REXLIB_APPLE || REXLIB_LINUX
		return get_asset_root() + "/plugins/lib" + name + ".so";
	#else
		#error "Unknown platform"
	#endif
}

} // namespace rexlib
