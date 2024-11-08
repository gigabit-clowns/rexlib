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

#include "cuda_device_memory_allocator.hpp"

#include "cuda_device_queue.hpp"
#include "default_cuda_device_buffer.hpp"

#include <stdexcept>

#include <cuda_runtime.h>

namespace xmipp4
{
namespace compute
{

cuda_device_memory_allocator::cuda_device_memory_allocator(int device_id)
    : m_allocator(device_id)
{
}

std::unique_ptr<device_buffer> 
cuda_device_memory_allocator::create_buffer(numerical_type type, 
                                            std::size_t count,
                                            device_queue &queue )
{
    auto &cuda_queue = dynamic_cast<cuda_device_queue&>(queue);
    const auto &block = allocate(type, count, cuda_queue);
    return std::make_unique<default_cuda_device_buffer>(
        type, count,
        block, *this, cuda_queue
    );
}

std::shared_ptr<device_buffer> 
cuda_device_memory_allocator::create_buffer_shared(numerical_type type, 
                                                   std::size_t count,
                                                   device_queue &queue )
{
    auto &cuda_queue = dynamic_cast<cuda_device_queue&>(queue);
    const auto &block = allocate(type, count, cuda_queue);
    return std::make_shared<default_cuda_device_buffer>(
        type, count,
        block, *this, cuda_queue
    );
}

const cuda_memory_block&
cuda_device_memory_allocator::allocate(numerical_type type, 
                                       std::size_t count,
                                       cuda_device_queue& )
{
    const auto size = count * get_size(type);
    const auto *block = m_cache.allocate(m_allocator, size);

    if(!block)
    {
        throw std::bad_alloc();
    }

    return *block;
}

void cuda_device_memory_allocator::deallocate(const cuda_memory_block &block,
                                              cuda_device_queue& )
{
    m_cache.deallocate(block);
}

} // namespace compute
} // namespace xmipp4
