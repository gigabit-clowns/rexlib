// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "allocator/cuda_host_malloc.hpp"
#include "allocator/cuda_caching_memory_allocator.hpp"

#include <xmipp4/core/compute/host_memory_allocator.hpp>

#include <mutex>

namespace xmipp4 
{
namespace compute
{

class cuda_device_queue;

class cuda_host_memory_allocator
    : public host_memory_allocator
{
public:
    cuda_host_memory_allocator();
    cuda_host_memory_allocator(const cuda_host_memory_allocator &other) = delete;
    cuda_host_memory_allocator(cuda_host_memory_allocator &&other) = delete;
    ~cuda_host_memory_allocator() override = default;

    cuda_host_memory_allocator&
    operator=(const cuda_host_memory_allocator &other) = delete;
    cuda_host_memory_allocator&
    operator=(cuda_host_memory_allocator &&other) = delete;

    std::shared_ptr<host_buffer> 
    create_host_buffer(std::size_t size,
                       std::size_t alignment, 
                       device_queue &queue ) override;

    std::shared_ptr<host_buffer> 
    create_host_buffer(std::size_t size,
                       std::size_t alignment, 
                       cuda_device_queue &queue );

    std::shared_ptr<host_buffer> 
    create_host_buffer(std::size_t size, std::size_t alignment) override;
    
    const cuda_memory_block& allocate(std::size_t size,
                                      std::size_t alignment,
                                      cuda_device_queue *queue,
                                      cuda_memory_block_usage_tracker **usage_tracker );
    void deallocate(const cuda_memory_block &block);

private:
    cuda_caching_memory_allocator<cuda_host_malloc> m_allocator;
    std::mutex m_mutex;

}; 

} // namespace compute
} // namespace xmipp4
