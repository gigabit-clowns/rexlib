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
 * @file cuda_memory_block_cache.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_memory_block_cache.hpp
 * @date 2024-11-28
 * 
 */

#include "cuda_memory_block_cache.hpp"

namespace xmipp4
{
namespace compute
{

inline
cuda_memory_block_cache::cuda_memory_block_cache(std::size_t minimum_size, 
                                                 std::size_t request_size_step )
    : m_minimum_size(minimum_size)
    , m_request_size_step(request_size_step)
{
}

template <typename Allocator>
inline
void cuda_memory_block_cache::release(Allocator &allocator)
{
    m_deferred_blocks.process_pending_free(m_block_pool);
    release_blocks(m_block_pool, allocator);
}

template <typename Allocator>
inline
const cuda_memory_block* 
cuda_memory_block_cache::allocate(Allocator &allocator, 
                                  std::size_t size, 
                                  std::size_t queue_id ) 
{
    m_deferred_blocks.process_pending_free(m_block_pool);

    return allocate_block(
        m_block_pool,
        allocator, 
        size,
        queue_id,
        m_minimum_size,
        m_request_size_step
    );
}

inline
void cuda_memory_block_cache::deallocate(const cuda_memory_block &block,
                                         span<cuda_device_queue*> other_queues )
{
    if (other_queues.empty())
    {
        deallocate_block(m_block_pool, block);
    }
    else
    {
        m_deferred_blocks.record_events(block, other_queues);
    }
}

} // namespace compute
} // namespace xmipp4
