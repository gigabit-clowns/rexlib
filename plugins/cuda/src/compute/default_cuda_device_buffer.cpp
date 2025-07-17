// SPDX-License-Identifier: GPL-3.0-only

#include "default_cuda_device_buffer.hpp"

#include <xmipp4/cuda/compute/allocator/cuda_memory_block.hpp>
#include <xmipp4/cuda/compute/allocator/cuda_memory_block_usage_tracker.hpp>
#include <xmipp4/cuda/compute/cuda_device_queue.hpp>
#include <xmipp4/cuda/compute/cuda_device_memory_allocator.hpp>

namespace xmipp4
{
namespace compute
{

default_cuda_device_buffer
::default_cuda_device_buffer(std::size_t size,
                             std::size_t alignment,
                             cuda_device_queue *queue,
                             cuda_device_memory_allocator &allocator) noexcept
    : m_size(size)
    , m_block(allocate(size, alignment, queue, allocator, &m_usage_tracker))
{
}

std::size_t default_cuda_device_buffer::get_size() const noexcept
{
    return m_size;
}

void* default_cuda_device_buffer::get_data() noexcept
{
    return m_block ? m_block->get_data() : nullptr;
}

const void* default_cuda_device_buffer::get_data() const noexcept
{
    return m_block ? m_block->get_data() : nullptr;
}

host_buffer* default_cuda_device_buffer::get_host_accessible_alias() noexcept
{
    return nullptr;
}

const host_buffer* 
default_cuda_device_buffer::get_host_accessible_alias() const noexcept
{
    return nullptr;
}

void default_cuda_device_buffer::record_queue(device_queue &queue)
{
    record_queue(dynamic_cast<cuda_device_queue&>(queue));
}

void default_cuda_device_buffer::record_queue(cuda_device_queue &queue)
{
    m_usage_tracker->add_queue(*m_block, queue);
}



std::unique_ptr<const cuda_memory_block, default_cuda_device_buffer::block_delete>
default_cuda_device_buffer::allocate(std::size_t size,
                                     std::size_t alignment,
                                     cuda_device_queue *queue,
                                     cuda_device_memory_allocator &allocator,
                                     cuda_memory_block_usage_tracker **usage_tracker )
{
    const auto &block = allocator.allocate(
        size, 
        alignment, 
        queue, 
        usage_tracker
    );

    return std::unique_ptr<const cuda_memory_block, block_delete>(
        &block,
        block_delete(allocator)
    );
}

} // namespace compute
} // namespace xmipp4
