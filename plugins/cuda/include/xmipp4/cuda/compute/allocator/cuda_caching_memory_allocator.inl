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
 * @file cuda_deferred_release.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_deferred_release.hpp
 * @date 2024-11-28
 * 
 */

#include "cuda_caching_memory_allocator.hpp"

#include <new>

namespace xmipp4
{
namespace compute
{

template <typename Allocator>
inline
cuda_caching_memory_allocator<Allocator>
::cuda_caching_memory_allocator(allocator_type allocator,
                                std::size_t minimum_size, 
                                std::size_t request_size_step )
    : m_allocator(std::move(allocator))
    , m_cache(minimum_size, request_size_step)
{
}
    
template <typename Allocator>
inline
void cuda_caching_memory_allocator<Allocator>::release()
{
    m_cache.release(m_allocator);
}

template <typename Allocator>
inline
cuda_memory_block&
cuda_caching_memory_allocator<Allocator>
::allocate(std::size_t size, const cuda_device_queue *queue)
{
    auto *result = m_cache.allocate(m_allocator, size, queue);
    if (!result)
    {
        // Re-added retrial
        m_cache.release(m_allocator);
        result = m_cache.allocate(m_allocator, size, queue);
    }

    if (!result)
    {
        throw std::bad_alloc();
    }

    return *result;
}

template <typename Allocator>
inline
void cuda_caching_memory_allocator<Allocator>
::deallocate(const cuda_memory_block &block)
{
    m_cache.deallocate(block);
}
    
} // namespace compute
} // namespace xmipp4
