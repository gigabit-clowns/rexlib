// SPDX-License-Identifier: GPL-3.0-only


#include <catch2/catch_test_macros.hpp>

#include <xmipp4/core/plugin_manager.hpp>
#include <xmipp4/core/plugin.hpp>
#include <xmipp4/core/platform/operating_system.h>

using namespace xmipp4;


static std::string get_cuda_plugin_path()
{
	#if XMIPP4_WINDOWS
		return "xmipp4-hardware-cuda.dll";
	#elif XMIPP4_LINUX
		return "./libxmipp4-hardware-cuda.so";
	#elif XMIPP4_APPLE
		return "./libxmipp4-hardware-cuda.dylib";
	#else
		#error "Unknown platform"
	#endif
}

TEST_CASE( "load and register xmipp4-hardware-cuda plugin", "[hardware-cuda]" ) 
{
	plugin_manager manager;

	const auto* cuda_plugin = 
		manager.load_plugin(get_cuda_plugin_path());

	REQUIRE( cuda_plugin != nullptr );
	REQUIRE( cuda_plugin->get_name() == "xmipp4-hardware-cuda" );
}
