// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/platform/operating_system.h>
#include <xmipp4/core/platform/compiler.h>

#include <string>

#ifndef REX_TEST_ASSET_ROOT
	#error "REX_TEST_ASSET_ROOT is not defined. Link against " \
	       "xmipp4-test-assets-interface to consume the shared test assets."
#endif

namespace xmipp4
{

inline std::string get_asset_root()
{
	return REX_TEST_ASSET_ROOT;
}

inline std::string get_text_file_path()
{
	#if REX_WINDOWS
		return get_asset_root() + "\\" + "lorem_ipsum.txt";
	#elif REX_APPLE || REX_LINUX
		return get_asset_root() + "/" + "lorem_ipsum.txt";
	#else
		#error "Unknown platform"
	#endif
}

inline std::string get_test_plugin_directory()
{
	#if REX_WINDOWS
		return get_asset_root() + "\\plugins";
	#elif REX_APPLE || REX_LINUX
		return get_asset_root() + "/plugins";
	#else
		#error "Unknown platform"
	#endif
}

inline std::string get_mock_plugin_path(const std::string &name)
{

	#if REX_WINDOWS
		#if REX_MINGW
			return get_asset_root() + "\\plugins\\lib" + name + ".dll";
		#else
			return get_asset_root() + "\\plugins\\" + name + ".dll";
		#endif
	#elif REX_APPLE || REX_LINUX
		return get_asset_root() + "/plugins/lib" + name + ".so";
	#else
		#error "Unknown platform"
	#endif
}

} // namespace xmipp4
