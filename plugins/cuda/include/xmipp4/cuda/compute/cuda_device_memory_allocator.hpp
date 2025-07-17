// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "allocator/cuda_device_malloc.hpp"
#include "allocator/cuda_caching_memory_allocator.hpp"

#include <xmipp4/core/compute/device_memory_allocator.hpp>
#include <xmipp4/core/span.hpp>

#include <map>
#include <set>
#include <forward_list>

namespace xmipp4 
{
namespace compute
{

class cuda_device;
class cuda_device_queue;
class cuda_device_buffer;
class cuda_event;

class cuda_device_memory_allocator
    : public device_memory_allocator
{
public:
    explicit cuda_device_memory_allocator(cuda_device &device);
    cuda_device_memory_allocator(const cuda_device_memory_allocator &other) = delete;
    cuda_device_memory_allocator(cuda_device_memory_allocator &&other) = default;
    ~cuda_device_memory_allocator() override = default;

    cuda_device_memory_allocator&
    operator=(const cuda_device_memory_allocator &other) = delete;
    cuda_device_memory_allocator&
    operator=(cuda_device_memory_allocator &&other) = default;

    std::shared_ptr<device_buffer> 
    create_device_buffer(std::size_t size,
                         std::size_t alignment,
                         device_queue &queue ) override;

    std::shared_ptr<cuda_device_buffer> 
    create_device_buffer(std::size_t size,
                         std::size_t alignment,
                         cuda_device_queue &queue );

    const cuda_memory_block& allocate(std::size_t size,
                                      std::size_t alignment,
                                      cuda_device_queue *queue,
                                      cuda_memory_block_usage_tracker **usage_tracker );
    void deallocate(const cuda_memory_block &block);

private:
    cuda_caching_memory_allocator<cuda_device_malloc> m_allocator;

}; 

} // namespace compute
} // namespace xmipp4
