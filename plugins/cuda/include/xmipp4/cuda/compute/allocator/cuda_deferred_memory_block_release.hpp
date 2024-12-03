#pragma once

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
 * @file cuda_deferred_memory_block_release.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines data structure representing a memory cache.
 * @date 2024-11-28
 * 
 */

#include "cuda_memory_block.hpp"
#include "cuda_memory_block_pool.hpp"
#include "../cuda_event.hpp"
#include "../cuda_device_queue.hpp"

#include <xmipp4/core/span.hpp>

#include <forward_list>
#include <utility>
#include <vector>

namespace xmipp4 
{
namespace compute
{

/**
 * @brief Handles deferred deallocations to allow mixing multiple
 * CUDA streams.
 * 
 */
class cuda_deferred_memory_block_release
{
public:
    cuda_deferred_memory_block_release() = default;
    cuda_deferred_memory_block_release(const cuda_deferred_memory_block_release &other) = delete;
    cuda_deferred_memory_block_release(cuda_deferred_memory_block_release &&other) = default;
    ~cuda_deferred_memory_block_release() = default;

    cuda_deferred_memory_block_release&
    operator=(const cuda_deferred_memory_block_release &other) = delete;
    cuda_deferred_memory_block_release&
    operator=(cuda_deferred_memory_block_release &&other) = default;

    /**
     * @brief Iterate through release events and return back all blocks
     * that have no pending events.
     * 
     * @param cache The cache from which all blocks were allocated.
     * 
     */
    void process_pending_free(cuda_memory_block_pool &cache);

    /**
     * @brief Record events for each of the provided CUDA streams for a 
     * given block.
     * 
     * @param ite Iterator to the block. Must be dereferenceable.
     * @param other_queues Queues that need to be processed for actually
     * freeing the block.
     * 
     * @note This function does not check wether ite has been provided 
     * previously. Calling it twice with the same block before it has
     * been returned to the pool leads to undefined behavior.
     * 
     */
    void record_events(cuda_memory_block_pool::iterator ite,
                       span<cuda_device_queue *const> other_queues );

private:
    using event_list = std::forward_list<cuda_event>;

    event_list m_event_pool;
    std::vector<std::pair<cuda_memory_block_pool::iterator, event_list>> m_pending_free;

    /**
     * @brief Pop all signaled events from the list.
     * 
     * @param events Event list from which completed events are popt.
     */
    void pop_completed_events(event_list &events);

}; 

} // namespace compute
} // namespace xmipp4

#include "cuda_deferred_memory_block_release.inl"
