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
 * @file cuda_device.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_device.hpp
 * @date 2024-10-30
 * 
 */

#include <xmipp4/cuda/compute/cuda_device.hpp>

#include <xmipp4/cuda/compute/cuda_device_queue.hpp>
#include <xmipp4/cuda/compute/cuda_device_memory_allocator.hpp>
#include <xmipp4/cuda/compute/cuda_host_memory_allocator.hpp>
#include <xmipp4/cuda/compute/cuda_device_to_host_transfer.hpp>
#include <xmipp4/cuda/compute/cuda_host_to_device_transfer.hpp>
#include <xmipp4/cuda/compute/cuda_device_copy.hpp>
#include <xmipp4/cuda/compute/cuda_event.hpp>

#include <xmipp4/core/compute/device_create_parameters.hpp>

#include <memory>

namespace xmipp4
{
namespace compute
{

cuda_device::cuda_device(int device, const device_create_parameters &params)
    : m_device(device)
    , m_queue_pool(device, params.get_desired_queue_count())
{
}

int cuda_device::get_index() const noexcept
{
    return m_device;
}

cuda_device_queue_pool& cuda_device::get_queue_pool()
{
    return m_queue_pool;
}

std::shared_ptr<device_memory_allocator> 
cuda_device::create_device_memory_allocator()
{
    return std::make_shared<cuda_device_memory_allocator>(*this);
}

std::shared_ptr<host_memory_allocator> 
cuda_device::create_host_memory_allocator()
{
    return std::make_shared<cuda_host_memory_allocator>();
}

std::shared_ptr<host_to_device_transfer> 
cuda_device::create_host_to_device_transfer()
{
    return std::make_shared<cuda_host_to_device_transfer>();
}

std::shared_ptr<device_to_host_transfer> 
cuda_device::create_device_to_host_transfer()
{
    return std::make_shared<cuda_device_to_host_transfer>();
}

std::shared_ptr<device_copy> 
cuda_device::create_device_copy()
{
    return std::make_shared<cuda_device_copy>();
}

std::shared_ptr<device_event> cuda_device::create_device_event()
{
    return std::make_shared<cuda_event>();
}

std::shared_ptr<device_to_host_event> 
cuda_device::create_device_to_host_event()
{
    return std::make_shared<cuda_event>();
}

} // namespace compute
} // namespace xmipp4
