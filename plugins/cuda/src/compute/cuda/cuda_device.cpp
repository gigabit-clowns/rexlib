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

#include "cuda_device.hpp"

#include "cuda_device_queue.hpp"
#include "cuda_device_memory_allocator.hpp"
#include "cuda_host_memory_allocator.hpp"

#include <memory>

namespace xmipp4
{
namespace compute
{

cuda_device::cuda_device(int device)
    : m_device(device)
{
}

std::unique_ptr<device_queue> cuda_device::create_queue()
{
    return std::make_unique<cuda_device_queue>(m_device);
}

std::shared_ptr<device_queue> cuda_device::create_queue_shared()
{
    return std::make_shared<cuda_device_queue>(m_device);
}

std::unique_ptr<device_memory_pool> 
cuda_device::create_device_memory_pool()
{
    return std::make_unique<cuda_device_memory_pool>();
}

std::shared_ptr<device_memory_pool> 
cuda_device::create_device_memory_pool_shared()
{
    return std::make_shared<cuda_device_memory_pool>();
}

std::unique_ptr<host_memory_pool> 
cuda_device::create_host_memory_pool()
{
    return std::make_unique<cuda_host_memory_pool>();
}

std::shared_ptr<host_memory_pool> 
cuda_device::create_host_memory_pool_shared()
{
    return std::make_shared<cuda_host_memory_pool>();
}

} // namespace compute
} // namespace xmipp4
