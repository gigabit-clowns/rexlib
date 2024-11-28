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
 * @file cuda_device_memory_allocator.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_device_memory_allocator.hpp
 * @date 2024-11-06
 * 
 */

#include <xmipp4/cuda/compute/cuda_device_memory_allocator.hpp>

#include "default_cuda_device_buffer.hpp"

#include <xmipp4/cuda/compute/cuda_device.hpp>
#include <xmipp4/cuda/compute/cuda_device_queue.hpp>
#include <xmipp4/cuda/compute/cuda_event.hpp>

#include <stdexcept>

#include <cuda_runtime.h>

namespace xmipp4
{
namespace compute
{

cuda_device_memory_allocator::cuda_device_memory_allocator(cuda_device &device)
    : m_allocator(cuda_device_malloc(device.get_index()), 512, 2<<20)
{
}

std::unique_ptr<device_buffer> 
cuda_device_memory_allocator::create_buffer(numerical_type type, 
                                            std::size_t count,
                                            device_queue &queue )
{
    return create_buffer(
        type, count, dynamic_cast<cuda_device_queue&>(queue)
    );
}

std::shared_ptr<device_buffer> 
cuda_device_memory_allocator::create_buffer_shared(numerical_type type, 
                                                   std::size_t count,
                                                   device_queue &queue )
{
    return create_buffer_shared(
        type, count, dynamic_cast<cuda_device_queue&>(queue)
    );
}
std::unique_ptr<cuda_device_buffer> 
cuda_device_memory_allocator::create_buffer(numerical_type type, 
                                            std::size_t count, 
                                            cuda_device_queue &queue )
{
    const auto &block = allocate(type, count, queue);
    return std::make_unique<default_cuda_device_buffer>(
        type, count, block, *this
    );
}

std::shared_ptr<cuda_device_buffer> 
cuda_device_memory_allocator::create_buffer_shared(numerical_type type, 
                                                   std::size_t count, 
                                                   cuda_device_queue &queue )
{
    const auto &block = allocate(type, count, queue);
    return std::make_shared<default_cuda_device_buffer>(
       type, count, block, *this
    );
}

const cuda_memory_block&
cuda_device_memory_allocator::allocate(numerical_type type, 
                                       std::size_t count,
                                       cuda_device_queue& queue )
{
    const auto size = count * get_size(type);
    const auto queue_id = queue.get_id();
    const auto *block = m_allocator.allocate(size, queue_id);
    if(!block)
    {
        throw std::bad_alloc();
    }

    return *block;
}

void cuda_device_memory_allocator::deallocate(const cuda_memory_block &block,
                                              span<cuda_device_queue*> other_queues )
{
    m_allocator.deallocate(block, other_queues);
}

} // namespace compute
} // namespace xmipp4
