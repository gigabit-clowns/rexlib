// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/cuda/compute/cuda_host_memory_allocator.hpp>

#include "default_cuda_host_buffer.hpp"

#include <xmipp4/cuda/compute/cuda_device_queue.hpp>

namespace xmipp4
{
namespace compute
{

XMIPP4_CONST_CONSTEXPR
std::size_t XMIPP4_CUDA_HOST_MEMORY_REQUEST_ROUND_STEP = 512;
XMIPP4_CONST_CONSTEXPR
std::size_t XMIPP4_CUDA_HOST_MEMORY_ALLOCATE_ROUND_STEP = 2<<20; // 2MB

cuda_host_memory_allocator::cuda_host_memory_allocator()
    : m_allocator(
        {}, 
        XMIPP4_CUDA_HOST_MEMORY_REQUEST_ROUND_STEP, 
        XMIPP4_CUDA_HOST_MEMORY_ALLOCATE_ROUND_STEP
    )
{
}

std::shared_ptr<host_buffer> 
cuda_host_memory_allocator::create_host_buffer(std::size_t size, 
                                               std::size_t alignment,
                                               device_queue &queue )
{
    return create_host_buffer(
        size,
        alignment, 
        dynamic_cast<cuda_device_queue&>(queue)
    );
}

std::shared_ptr<host_buffer> 
cuda_host_memory_allocator::create_host_buffer(std::size_t size, 
                                               std::size_t alignment,
                                               cuda_device_queue &queue )
{
    return std::make_shared<default_cuda_host_buffer>(
        size, 
        alignment, 
        &queue, 
        *this
    );
}

std::shared_ptr<host_buffer> 
cuda_host_memory_allocator::create_host_buffer(std::size_t size, 
                                               std::size_t alignment )
{
    return std::make_shared<default_cuda_host_buffer>(
        size, 
        alignment, 
        nullptr,
        *this
    );
}

const cuda_memory_block& 
cuda_host_memory_allocator::allocate(std::size_t size,
                                     std::size_t alignment,
                                     cuda_device_queue *queue,
                                     cuda_memory_block_usage_tracker **usage_tracker )
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_allocator.allocate(size, alignment, queue, usage_tracker);
}

void cuda_host_memory_allocator::deallocate(const cuda_memory_block &block)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_allocator.deallocate(block);
}

} // namespace compute
} // namespace xmipp4
