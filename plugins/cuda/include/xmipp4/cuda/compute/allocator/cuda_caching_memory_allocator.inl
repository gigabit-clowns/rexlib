// SPDX-License-Identifier: GPL-3.0-only

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
const cuda_memory_block&
cuda_caching_memory_allocator<Allocator>
::allocate(std::size_t size, 
           std::size_t alignment,
           const cuda_device_queue *queue, 
           cuda_memory_block_usage_tracker **usage_tracker )
{
    auto *result = m_cache.allocate(
        m_allocator, 
        size, 
        alignment, 
        queue, 
        usage_tracker
    );

    if (!result)
    {
        // Retry after releasing blocks
        m_cache.release(m_allocator);
        result = m_cache.allocate(
            m_allocator, 
            size, 
            alignment, 
            queue, 
            usage_tracker
        );
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
