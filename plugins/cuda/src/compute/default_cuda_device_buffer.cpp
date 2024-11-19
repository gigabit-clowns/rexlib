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


default_cuda_device_buffer::default_cuda_device_buffer() noexcept
    : m_type(numerical_type::unknown)
    , m_count(0)
    , m_block(nullptr)
    , m_allocator(nullptr)
{
}

default_cuda_device_buffer
::default_cuda_device_buffer(numerical_type type,
                             std::size_t count,
                             const cuda_memory_block &block , 
                             cuda_device_memory_allocator &allocator ) noexcept
    : m_type(type)
    , m_count(count)
    , m_block(&block)
    , m_allocator(&allocator)
{
}

default_cuda_device_buffer
::default_cuda_device_buffer(default_cuda_device_buffer &&other) noexcept
    : m_type(other.m_type)
    , m_count(other.m_count)
    , m_block(other.m_block)
    , m_allocator(other.m_allocator)
    , m_queues(std::move(other.m_queues))
{
    other.m_type = numerical_type::unknown;
    other.m_count = 0;
    other.m_block = nullptr;
    other.m_allocator = nullptr;
}

default_cuda_device_buffer::~default_cuda_device_buffer()
{
    reset();
}

default_cuda_device_buffer& 
default_cuda_device_buffer::operator=(default_cuda_device_buffer &&other) noexcept
{
    swap(other);
    other.reset();
    return *this;
}

void default_cuda_device_buffer::swap(default_cuda_device_buffer &other) noexcept
{
    std::swap(m_type, other.m_type);
    std::swap(m_count, other.m_count);
    std::swap(m_block, other.m_block);
    std::swap(m_allocator, other.m_allocator);
}

void default_cuda_device_buffer::reset() noexcept
{
    if (m_block)
    {
        XMIPP4_ASSERT(m_allocator);
        m_allocator->deallocate(*m_block, make_span(m_queues));

        m_type = numerical_type::unknown;
        m_count = 0UL;
        m_block = nullptr;
        m_allocator = nullptr;
        m_queues.clear();
    }
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


void default_cuda_device_buffer::record_queue(cuda_device_queue &queue)
{
    if (queue.get_id() != m_block->get_queue_id())
    {
        auto *queue_pointer = &queue;
        const auto pos = std::lower_bound(
            m_queues.cbegin(), 
            m_queues.cend(),
            queue_pointer
        );

        if (pos == m_queues.cend() || *pos != queue_pointer)
        {
            m_queues.insert(std::next(pos), queue_pointer);
        }
    }
}

} // namespace compute
} // namespace xmipp4
