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
 * @file cuda_deferred_memory_block_release.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_deferred_memory_block_release.hpp
 * @date 2024-11-28
 * 
 */

#include "cuda_deferred_memory_block_release.hpp"

namespace xmipp4
{
namespace compute
{

void cuda_deferred_memory_block_release::process_pending_free(cuda_memory_block_pool &cache)
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
            deallocate_block(cache, ite->first);
            ite = m_pending_free.erase(ite);
        }
        else
        {
            ++ite;
        }
    }
}

void cuda_deferred_memory_block_release::record_events(const cuda_memory_block &block,
                                                       span<cuda_device_queue*> queues )
{
    decltype(m_pending_free)::iterator ite;
    std::tie(ite, std::ignore) = m_pending_free.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(block),
        std::forward_as_tuple()
    );

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

        events.front().signal(*queue);
    }
}

void cuda_deferred_memory_block_release::pop_completed_events(event_list &events)
{
    auto prev_ite = events.cbefore_begin();
    event_list::const_iterator ite;
    while ((ite = std::next(prev_ite)) != events.cend())
    {
        if(ite->is_signaled())
        {
            // Return the event to the pool
            m_event_pool.splice_after(
                m_event_pool.cbefore_begin(),
                events,
                prev_ite
            );
        }
        else
        {
            ++prev_ite;
        }
    }
}

} // namespace compute
} // namespace xmipp4
