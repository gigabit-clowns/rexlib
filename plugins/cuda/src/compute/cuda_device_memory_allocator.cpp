// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/cuda/compute/cuda_device_memory_allocator.hpp>

#include "default_cuda_device_buffer.hpp"

#include <xmipp4/cuda/compute/cuda_device.hpp>
#include <xmipp4/cuda/compute/cuda_device_queue.hpp>
#include <xmipp4/core/platform/constexpr.hpp>

namespace xmipp4
{
namespace compute
{

XMIPP4_CONST_CONSTEXPR
std::size_t XMIPP4_CUDA_DEVICE_MEMORY_REQUEST_ROUND_STEP = 512;
XMIPP4_CONST_CONSTEXPR
std::size_t XMIPP4_CUDA_DEVICE_MEMORY_ALLOCATE_ROUND_STEP = 2<<20; // 2MB

cuda_device_memory_allocator::cuda_device_memory_allocator(cuda_device &device)
    : m_allocator(
        cuda_device_malloc(device.get_index()), 
        XMIPP4_CUDA_DEVICE_MEMORY_REQUEST_ROUND_STEP, 
        XMIPP4_CUDA_DEVICE_MEMORY_ALLOCATE_ROUND_STEP
    )
{
}

std::shared_ptr<device_buffer> 
cuda_device_memory_allocator::create_device_buffer(std::size_t size, 
                                                   std::size_t alignment,
                                                   device_queue &queue )
{
    return create_device_buffer(
        size, 
        alignment,
        dynamic_cast<cuda_device_queue&>(queue)
    );
}

std::shared_ptr<cuda_device_buffer> 
cuda_device_memory_allocator::create_device_buffer(std::size_t size, 
                                                   std::size_t alignment,
                                                   cuda_device_queue &queue )
{
    return std::make_shared<default_cuda_device_buffer>(
        size,
        alignment,
        &queue,
        *this
    );
}

const cuda_memory_block&
cuda_device_memory_allocator::allocate(std::size_t size,
                                       std::size_t alignment,
                                       cuda_device_queue *queue,
                                       cuda_memory_block_usage_tracker **usage_tracker)
{
    return m_allocator.allocate(size, alignment, queue, usage_tracker);
}

void cuda_device_memory_allocator::deallocate(const cuda_memory_block &block)
{
    m_allocator.deallocate(block);
}

} // namespace compute
} // namespace xmipp4
