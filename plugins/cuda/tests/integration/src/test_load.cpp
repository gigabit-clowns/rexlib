/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

/**
 * @file test_load.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Test that the plugin loads and registers.
 * @date 2024-10-30
 */


#include <catch2/catch_test_macros.hpp>

#include <xmipp4/core/plugin_manager.hpp>
#include <xmipp4/core/plugin.hpp>
#include <xmipp4/core/platform/operating_system.h>

using namespace xmipp4;


static std::string get_cuda_plugin_path()
{
    #if XMIPP4_WINDOWS
        return "xmipp4-compute-cuda.dll";
    #elif XMIPP4_APPLE || XMIPP4_LINUX
        return "./libxmipp4-compute-cuda.so";
    #else
        #error "Unknown platform"
    #endif
}

TEST_CASE( "load and register xmipp4-compute-cuda plugin", "[compute-cuda]" ) 
{
    plugin_manager manager;

    const auto* cuda_plugin = 
        manager.load_plugin(get_cuda_plugin_path());

    REQUIRE( cuda_plugin != nullptr );
    REQUIRE( cuda_plugin->get_name() == "xmipp4-compute-cuda" );
}
