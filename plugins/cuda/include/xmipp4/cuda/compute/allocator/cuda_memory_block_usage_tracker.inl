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
 * @file cuda_memory_block_usage_tracker.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_memory_block_usage_tracker.hpp
 * @date 2024-11-30
 * 
 */

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
