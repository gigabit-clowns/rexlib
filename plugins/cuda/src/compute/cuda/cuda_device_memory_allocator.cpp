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
#include "cuda_device_event.hpp"
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
        type, count, block, *this
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
       type, count, block, *this
    );
}

const cuda_memory_block&
cuda_device_memory_allocator::allocate(numerical_type type, 
                                       std::size_t count,
                                       cuda_device_queue& queue )
{
    process_pending_free();

    const auto size = count * get_size(type);
    const auto queue_id = queue.get_id();
    const auto *block = m_cache.allocate(m_allocator, size, queue_id);

    if(!block)
    {
        throw std::bad_alloc();
    }

    return *block;
}

void cuda_device_memory_allocator::deallocate(const cuda_memory_block &block,
                                              span<cuda_device_queue*> other_queues )
{
    if (other_queues.empty())
    {
        m_cache.deallocate(block);
    }
    else
    {
        record_events(block, other_queues);
    }
}


void cuda_device_memory_allocator::process_pending_free()
{
    auto ite = m_pending_free.begin();
    while (ite != m_pending_free.end())
    {
        // Remove all completed events
        auto &events = ite->second;
        pop_completed_events(events);

        // Return block if completed
        if(events.empty())
        {
            m_cache.deallocate(ite->first);
            ite = m_pending_free.erase(ite);
        }
        else
        {
            ++ite;
        }
    }
}

void cuda_device_memory_allocator
::record_events(const cuda_memory_block &block,
                span<cuda_device_queue*> queues)
{
    bool inserted;
    decltype(m_pending_free)::iterator ite;
    std::tie(ite, inserted) = m_pending_free.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(block),
        std::forward_as_tuple()
    );
    XMIPP4_ASSERT(inserted);

    auto& events = ite->second;
    for (cuda_device_queue *queue : queues)
    {
        // Add a new event to the front
        if (m_event_pool.empty())
        {
            events.emplace_front();
        }
        else
        {
            events.splice_after(
                events.cbefore_begin(),
                m_event_pool, 
                m_event_pool.cbefore_begin()
            );
        }

        events.front().record(*queue);
    }
}

void cuda_device_memory_allocator::pop_completed_events(event_list &events)
{
    auto prev_it = events.cbefore_begin();
    event_list::const_iterator ite;
    while ((ite = std::next(prev_it)) != events.cend())
    {
        if(ite->is_signaled())
        {
            // Return the event to the pool
            m_event_pool.splice_after(
                m_event_pool.cbefore_begin(),
                events,
                prev_it
            );
        }
        else
        {
            ++prev_it;
        }
    }
}

} // namespace compute
} // namespace xmipp4
