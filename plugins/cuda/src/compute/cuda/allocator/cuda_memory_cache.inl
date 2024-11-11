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
 * @file cuda_memory_cache.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_memory_cache.hpp
 * @date 2024-11-06
 * 
 */

#include "cuda_memory_cache.hpp"

namespace xmipp4
{
namespace compute
{

inline
cuda_memory_cache::cuda_memory_cache(std::size_t small_large_threshold,
                                     std::size_t size_step,
                                     std::size_t request_step )
    : m_small_large_threshold(small_large_threshold)
    , m_size_step(size_step)
    , m_request_size_step(request_step)
{
}

template <typename Allocator>
inline
void cuda_memory_cache::release(Allocator &allocator)
{
    release_blocks(m_small_block_pool, allocator);
    release_blocks(m_large_block_pool, allocator);
}

template <typename Allocator>
inline
const cuda_memory_block* 
cuda_memory_cache::allocate(Allocator &allocator, 
                            std::size_t size, 
                            std::size_t queue_id ) 
{
    const cuda_memory_block* result;

    memory::align_ceil_inplace(size, m_size_step);
    if (is_small(size))
    {
        result = allocate_from_pool(
            m_small_block_pool, allocator, 
            size, queue_id, m_size_step
        );
    }
    else
    {   
        result = allocate_from_pool(
            m_large_block_pool, allocator, 
            size, queue_id, m_small_large_threshold
        );
    }

    return result;
}

inline
void cuda_memory_cache::deallocate(const cuda_memory_block &block)
{
    if (is_small(block.get_size()))
    {
        deallocate_block(m_small_block_pool, block);
    }
    else
    {
        deallocate_block(m_large_block_pool, block);
    }
}



template <typename Allocator>
inline
const cuda_memory_block* 
cuda_memory_cache::allocate_from_pool(cuda_memory_block_pool &blocks, 
                                      Allocator &allocator, 
                                      std::size_t size,
                                      std::size_t queue_id,
                                      std::size_t min_size )
{
    const cuda_memory_block* result;

    result = allocate_block(
        blocks, allocator, 
        size, queue_id, min_size, m_request_size_step
    );

    if(!result)
    {
        // Retry after freeing space
        release(allocator);
        result = allocate_block(
            m_small_block_pool, allocator, 
            size, queue_id, min_size, m_request_size_step
        );
    }

    return result;
}


bool cuda_memory_cache::is_small(std::size_t size) const noexcept
{
    return size < m_small_large_threshold;
}

} // namespace compute
} // namespace xmipp4
