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

#include "allocator/cuda_memory_block.hpp"
#include "cuda_device_memory_allocator.hpp"
#include "cuda_device_event.hpp"

#include <xmipp4/core/platform/assert.hpp>

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
    , m_queue(nullptr)
{
}

default_cuda_device_buffer
::default_cuda_device_buffer(numerical_type type,
                             std::size_t count,
                             const cuda_memory_block &block , 
                             cuda_device_memory_allocator &allocator,
                             cuda_device_queue &queue) noexcept
    : m_type(type)
    , m_count(count)
    , m_block(&block)
    , m_allocator(&allocator)
    , m_queue(&queue)
{
}

default_cuda_device_buffer
::default_cuda_device_buffer(default_cuda_device_buffer &&other) noexcept
    : m_type(other.m_type)
    , m_count(other.m_count)
    , m_block(other.m_block)
    , m_allocator(other.m_allocator)
    , m_queue(other.m_queue)
    , m_events(std::move(other.m_events))
{
    other.m_type = numerical_type::unknown;
    other.m_count = 0;
    other.m_block = nullptr;
    other.m_allocator = nullptr;
    other.m_queue = nullptr;
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
    std::swap(m_queue, other.m_queue);
    std::swap(m_events, other.m_events);

}

void default_cuda_device_buffer::reset() noexcept
{
    if (m_block)
    {
        XMIPP4_ASSERT(m_allocator);
        m_allocator->deallocate(*m_block, *m_queue, std::move(m_events));

        m_type = numerical_type::unknown;
        m_count = 0UL;
        m_block = nullptr;
        m_allocator = nullptr;
        m_queue = nullptr;
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
    m_events.emplace_front();
    m_events.front().record(queue);
}

} // namespace compute
} // namespace xmipp4
