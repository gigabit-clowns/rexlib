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
 * @file default_cuda_device_buffer.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of default_cuda_device_buffer.hpp
 * @date 2024-10-30
 * 
 */

#include "default_cuda_device_buffer.hpp"

#include <xmipp4/cuda/compute/allocator/cuda_memory_block.hpp>
#include <xmipp4/cuda/compute/cuda_device_memory_allocator.hpp>
#include <xmipp4/cuda/compute/cuda_device_queue.hpp>

#include <xmipp4/core/platform/assert.hpp>

#include <algorithm>

#include <cuda_runtime.h>

namespace xmipp4
{
namespace compute
{

default_cuda_device_buffer
::default_cuda_device_buffer(numerical_type type,
                             std::size_t count,
                             cuda_memory_block &block , 
                             cuda_device_memory_allocator &allocator ) noexcept
    : m_type(type)
    , m_count(count)
    , m_block(&block, block_delete(allocator))
{
}

numerical_type default_cuda_device_buffer::get_type() const noexcept
{
    return m_type;
}

std::size_t default_cuda_device_buffer::get_count() const noexcept
{
    return m_count;
}

void* default_cuda_device_buffer::get_data() noexcept
{
    return m_block ? m_block->get_data() : nullptr;
}

const void* default_cuda_device_buffer::get_data() const noexcept
{
    return m_block ? m_block->get_data() : nullptr;
}

host_buffer* default_cuda_device_buffer::get_host_accessible_alias() noexcept
{
    return nullptr;
}

const host_buffer* 
default_cuda_device_buffer::get_host_accessible_alias() const noexcept
{
    return nullptr;
}
void default_cuda_device_buffer::record_queue(device_queue &queue)
{
    record_queue_impl(dynamic_cast<cuda_device_queue&>(queue));
}

void default_cuda_device_buffer::record_queue_impl(cuda_device_queue &queue)
{
    m_block->register_extra_queue(queue);
}

} // namespace compute
} // namespace xmipp4
