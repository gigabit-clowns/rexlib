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
 * @file cuda_device_backend.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_device_backend.hpp
 * @date 2024-10-30
 * 
 */

#include <xmipp4/cuda/compute/cuda_device_backend.hpp>

#include <xmipp4/cuda/compute/cuda_error.hpp>
#include <xmipp4/cuda/compute/cuda_device.hpp>

#include <xmipp4/core/compute/device_manager.hpp>

#include <numeric>
#include <sstream>
#include <iomanip>
#include <cstdlib>

#include <cuda_runtime.h>

namespace xmipp4
{
namespace compute
{

static std::string pci_id_to_string(int bus_id, int device_id, int domain_id)
{
    std::ostringstream oss;

    oss << std::setfill('0') << std::setw(2) << bus_id << ':';
    oss << std::setfill('0') << std::setw(2) << device_id << '.';
    oss << std::setw(1) << domain_id << '.';

    return oss.str();
}



std::string cuda_device_backend::get_name() const noexcept
{
    return "cuda";
}

version cuda_device_backend::get_version() const noexcept
{
    int cuda_version;
    XMIPP4_CUDA_CHECK( cudaRuntimeGetVersion(&cuda_version) );

    const auto major_div = std::div(cuda_version, 1000);
    const auto minor_div = std::div(major_div.rem, 10);

    return version(
        major_div.quot,
        minor_div.quot,
        minor_div.rem
    );
}

bool cuda_device_backend::is_available() const noexcept
{
    int count = 0;
    cudaGetDeviceCount(&count);
    return count > 0;
}

backend_priority cuda_device_backend::get_priority() const noexcept
{
    return backend_priority::normal;
}

void cuda_device_backend::enumerate_devices(std::vector<std::size_t> &ids) const
{
    int count;
    cudaGetDeviceCount(&count);
    
    ids.resize(count);
    std::iota(
        ids.begin(), ids.end(),
        static_cast<std::size_t>(0)
    );
}

bool cuda_device_backend::get_device_properties(std::size_t id, 
                                                device_properties &desc ) const
{
    int count;
    cudaGetDeviceCount(&count);

    const auto device = static_cast<int>(id);
    const auto result = device < count;
    if (result)
    {
        cudaDeviceProp prop;
        XMIPP4_CUDA_CHECK( cudaGetDeviceProperties(&prop, device) );

        // Convert
        const auto type = 
            prop.integrated ? device_type::integrated_gpu : device_type::gpu;
        auto location = pci_id_to_string(
            prop.pciBusID, 
            prop.pciDeviceID, 
            prop.pciDomainID
        );

        // Write
        desc.set_name(std::string(prop.name));
        desc.set_physical_location(std::move(location));
        desc.set_type(type);
        desc.set_total_memory_bytes(prop.totalGlobalMem);
    }

    return result;
}

std::unique_ptr<device> 
cuda_device_backend::create_device(std::size_t id,
                                   const device_create_parameters &params )
{
    int count;
    XMIPP4_CUDA_CHECK( cudaGetDeviceCount(&count) );
    if (static_cast<int>(id) >= count)
    {
        throw std::invalid_argument("Invalid device id");
    }

    return std::make_unique<cuda_device>(id, params);
}

std::shared_ptr<device> 
cuda_device_backend::create_device_shared(std::size_t id,
                                          const device_create_parameters &params )
{
    int count;
    XMIPP4_CUDA_CHECK( cudaGetDeviceCount(&count) );
    if (static_cast<int>(id) >= count)
    {
        throw std::invalid_argument("Invalid device id");
    }

    return std::make_shared<cuda_device>(id, params);
}

bool cuda_device_backend::register_at(device_manager &manager)
{
    return manager.register_backend(std::make_unique<cuda_device_backend>());
}

} // namespace compute
} // namespace xmipp4
