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
 * @file default_cuda_host_buffer.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of default_cuda_host_buffer.hpp
 * @date 2024-10-30
 * 
 */

#include "default_cuda_host_buffer.hpp"

#include <xmipp4/cuda/compute/allocator/cuda_memory_block.hpp>
#include <xmipp4/cuda/compute/allocator/cuda_memory_block_usage_tracker.hpp>
#include <xmipp4/cuda/compute/cuda_device_queue.hpp>
#include <xmipp4/cuda/compute/cuda_host_memory_allocator.hpp>

namespace xmipp4
{
namespace compute
{

default_cuda_host_buffer
::default_cuda_host_buffer(std::size_t size,
                           std::size_t alignment,
                           cuda_device_queue *queue,
                           cuda_host_memory_allocator &allocator) noexcept
    : m_size(size)
    , m_block(allocate(size, alignment, queue, allocator, &m_usage_tracker))
{
}

std::size_t default_cuda_host_buffer::get_size() const noexcept
{
    return m_size;
}

void* default_cuda_host_buffer::get_data() noexcept
{
    return m_block ? m_block->get_data() : nullptr;
}

const void* default_cuda_host_buffer::get_data() const noexcept
{
    return m_block ? m_block->get_data() : nullptr;
}

device_buffer* default_cuda_host_buffer::get_device_accessible_alias() noexcept
{
    return nullptr;
}

const device_buffer* 
default_cuda_host_buffer::get_device_accessible_alias() const noexcept
{
    return nullptr;
}
void default_cuda_host_buffer::record_queue(device_queue &queue)
{
    record_queue(dynamic_cast<cuda_device_queue&>(queue));
}

void default_cuda_host_buffer::record_queue(cuda_device_queue &queue)
{
    m_usage_tracker->record_queue(*m_block, queue);
}



std::unique_ptr<const cuda_memory_block, default_cuda_host_buffer::block_delete>
default_cuda_host_buffer::allocate(std::size_t size,
                                   std::size_t alignment,
                                   cuda_device_queue *queue,
                                   cuda_host_memory_allocator &allocator,
                                   cuda_memory_block_usage_tracker **usage_tracker )
{
    const auto &block = allocator.allocate(
        size, 
        alignment, 
        queue, 
        usage_tracker
    );

    return std::unique_ptr<const cuda_memory_block, block_delete>(
        &block,
        block_delete(allocator)
    );
}

} // namespace compute
} // namespace xmipp4
