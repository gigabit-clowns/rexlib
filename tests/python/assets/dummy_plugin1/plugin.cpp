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
 * @file plugin.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Exports a dummy plugin
 * @date 2025-04-07
 * 
 */

#include <xmipp4/core/plugin.hpp>
#include <xmipp4/core/platform/dynamic_shared_object.h>

#if defined(XMIPP4_DUMMY_PLUGIN1_EXPORTING)
    #define XMIPP4_DUMMY_PLUGIN1_API XMIPP4_EXPORT
#else
    #define XMIPP4_DUMMY_PLUGIN1_API XMIPP4_IMPORT
#endif

namespace xmipp4
{

static const std::string name = "dummy-plugin1";

class dummy_plugin1 final
    : public xmipp4::plugin
{
    const std::string& get_name() const noexcept final
    {
        return name;
    }

    version get_version() const noexcept final
    {
        return version(1, 2, 3);
    }

    void register_at(interface_catalog&) const
    {
        // NO-OP
    }
};

} // namespace xmipp4

static const xmipp4::dummy_plugin1 instance;

extern "C"
{
XMIPP4_DUMMY_PLUGIN1_API const xmipp4::plugin* xmipp4_get_plugin() 
{
    return &instance;
}
}
