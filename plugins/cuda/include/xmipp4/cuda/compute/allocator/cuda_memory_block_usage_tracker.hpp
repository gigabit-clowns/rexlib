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
 * @file cuda_memory_block_usage_tracker.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines data structure representing a memory block.
 * @date 2024-11-30
 * 
 */

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
