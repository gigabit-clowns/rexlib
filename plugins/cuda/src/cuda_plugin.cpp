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
 * @file cuda_plugin.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_plugin.hpp
 * @date 2024-10-30
 * 
 */

#include "cuda_plugin.hpp"

#include <xmipp4/cuda/compute/cuda_device_backend.hpp>

#include <xmipp4/core/interface_catalog.hpp>
#include <xmipp4/core/compute/device_manager.hpp>

namespace xmipp4 
{

const std::string cuda_plugin::name = "xmipp4-compute-cuda";

const std::string& cuda_plugin::get_name() const noexcept
{
    return name; 
}

version cuda_plugin::get_version() const noexcept
{
    return version(
        VERSION_MAJOR,
        VERSION_MINOR,
        VERSION_PATCH
    );
}

void cuda_plugin::register_at(interface_catalog& catalog) const
{
    compute::cuda_device_backend::register_at(
        catalog.get_backend_manager<compute::device_manager>()
    );
}

} // namespace xmipp4
