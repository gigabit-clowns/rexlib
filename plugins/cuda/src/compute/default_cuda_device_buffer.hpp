// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <xmipp4/cuda/compute/cuda_device_buffer.hpp>
#include <xmipp4/cuda/compute/allocator/cuda_memory_allocator_delete.hpp>

#include <memory>

namespace xmipp4 
{
namespace compute
{

class cuda_memory_block;
class cuda_device_queue;
class cuda_memory_block_usage_tracker;
class cuda_device_memory_allocator;


class default_cuda_device_buffer final
    : public cuda_device_buffer
{
public:
    default_cuda_device_buffer(std::size_t size,
                             std::size_t alignment,
                             cuda_device_queue *queue,
                             cuda_device_memory_allocator &allocator ) noexcept;
    default_cuda_device_buffer(const default_cuda_device_buffer &other) = delete;
    default_cuda_device_buffer(default_cuda_device_buffer &&other) = default;
    ~default_cuda_device_buffer() override = default;

    default_cuda_device_buffer& 
    operator=(const default_cuda_device_buffer &other) = delete;
    default_cuda_device_buffer& 
    operator=(default_cuda_device_buffer &&other) = default;


    std::size_t get_size() const noexcept override;

    void* get_data() noexcept override;
    const void* get_data() const noexcept override;

    host_buffer* get_host_accessible_alias() noexcept override;
    const host_buffer* get_host_accessible_alias() const noexcept override;

    void record_queue(device_queue &queue) override;
    void record_queue(cuda_device_queue &queue);

private:
    using block_delete = 
        cuda_memory_allocator_delete<cuda_device_memory_allocator>;

    std::size_t m_size;
    cuda_memory_block_usage_tracker *m_usage_tracker;
    std::unique_ptr<const cuda_memory_block, block_delete> m_block;

    static std::unique_ptr<const cuda_memory_block, block_delete>
    allocate(std::size_t size,
             std::size_t alignment,
             cuda_device_queue *queue,
             cuda_device_memory_allocator &allocator,
             cuda_memory_block_usage_tracker **usage_tracker );

}; 

} // namespace compute
} // namespace xmipp4
