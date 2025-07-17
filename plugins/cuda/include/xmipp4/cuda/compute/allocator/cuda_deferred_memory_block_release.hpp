// SPDX-License-Identifier: GPL-3.0-only

#pragma once
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
 * CUDA queues (streams).
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
     * @brief Signal events for each of the provided CUDA queues for a 
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
    void signal_events(cuda_memory_block_pool::iterator ite,
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
