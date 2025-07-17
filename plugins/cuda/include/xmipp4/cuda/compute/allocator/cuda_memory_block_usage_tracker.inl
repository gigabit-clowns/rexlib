// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_memory_block_usage_tracker.hpp"

#include <algorithm>

namespace xmipp4
{
namespace compute
{

inline
void cuda_memory_block_usage_tracker::reset() noexcept
{
    m_queues.clear();
}

inline
void cuda_memory_block_usage_tracker::add_queue(const cuda_memory_block &block,
                                                cuda_device_queue &queue )
{
    auto *const queue_pointer = &queue;
    if (queue_pointer != block.get_queue())
    {
        // Find first element that compares greater or EQUAL.
        const auto pos = std::lower_bound(
            m_queues.cbegin(), m_queues.cend(),
            queue_pointer
        );

        // Ensure that it is not equal.
        if (pos == m_queues.cend() || *pos != queue_pointer)
        {
            m_queues.insert(pos, queue_pointer);
        }
    }
}

inline
span<cuda_device_queue *const> 
cuda_memory_block_usage_tracker::get_queues() const noexcept
{
    return make_span(m_queues);
}

} // namespace compute
} // namespace xmipp4
