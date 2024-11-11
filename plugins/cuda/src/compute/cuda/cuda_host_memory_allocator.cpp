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
 * @file cuda_host_memory_allocator.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_host_memory_allocator.hpp
 * @date 2024-11-06
 * 
 */

#include "cuda_host_memory_allocator.hpp"

#include "default_cuda_host_buffer.hpp"

#include <stdexcept>

#include <cuda_runtime.h>

namespace xmipp4
{
namespace compute
{

std::unique_ptr<host_buffer> 
cuda_host_memory_allocator::create_buffer(numerical_type type, 
                                          std::size_t count )
{
    const auto &block = allocate(type, count);
    return std::make_unique<default_cuda_host_buffer>(
        type, count,
        block, *this
    );
}

std::shared_ptr<host_buffer> 
cuda_host_memory_allocator::create_buffer_shared(numerical_type type, 
                                                 std::size_t count )
{
    const auto &block = allocate(type, count);
    return std::make_shared<default_cuda_host_buffer>(
        type, count,
        block, *this
    );
}

const cuda_memory_block&
cuda_host_memory_allocator::allocate(numerical_type type, std::size_t count)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    const auto size = count * get_size(type);
    const auto *block = m_cache.allocate(m_allocator, size, 0);

    if(!block)
    {
        throw std::bad_alloc();
    }

    return *block;
}

void cuda_host_memory_allocator::deallocate(const cuda_memory_block &block)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cache.deallocate(block);
}

} // namespace compute
} // namespace xmipp4
