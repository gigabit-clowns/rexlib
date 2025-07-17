// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "cuda_memory_block.hpp"

#include <xmipp4/core/span.hpp>

#include <vector>

namespace xmipp4 
{
namespace compute
{

class cuda_device_queue;

/**
 * @brief Keeps track of the usage of a cuda_memory_block across various
 * CUDA queues/streams.
 * 
 */
class cuda_memory_block_usage_tracker
{
public: 
    /**
     * @brief Reset queue usage.
     * 
     * Deletes the queue inventory and leaves the tracker in
     * a newly created state.
     * 
     */
    void reset() noexcept;

    /**
     * @brief Add a queue to the inventory.
     * 
     * The queue is only added if it is different to owner of the
     * provided block and it has not been added previously.
     * 
     * @param block The cuda memory block.
     * @param queue Queue where the block has been used.
     */
    void add_queue(const cuda_memory_block &block, cuda_device_queue &queue);

    /**
     * @brief Get the list of queues where the block has been used.
     * 
     * @return span<cuda_device_queue *const> List of queues.
     */
    span<cuda_device_queue *const> get_queues() const noexcept;

private:
    std::vector<cuda_device_queue*> m_queues;

};

} // namespace compute
} // namespace xmipp4

#include "cuda_memory_block_usage_tracker.inl"
