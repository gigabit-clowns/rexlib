// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <xmipp4/core/compute/device_to_host_transfer.hpp>

#include "cuda_event.hpp"

namespace xmipp4 
{
namespace compute
{

class cuda_host_buffer;
class cuda_host_memory_allocator;
class cuda_device_buffer;
class cuda_device_queue;

/**
 * @brief CUDA implementation of the device to host transfer engine.
 * 
 */
class cuda_device_to_host_transfer final
    : public device_to_host_transfer
{
public:
    void transfer_copy(const device_buffer &src_buffer, 
                       host_buffer &dst_buffer, 
                       device_queue &queue ) override;

    void transfer_copy(const cuda_device_buffer &src_buffer, 
                       host_buffer &dst_buffer, 
                       cuda_device_queue &queue );

    void transfer_copy(const device_buffer &src_buffer,
                       host_buffer &dst_buffer,
                       span<const copy_region> regions,
                       device_queue &queue ) override;

    void transfer_copy(const cuda_device_buffer &src_buffer, 
                       host_buffer &dst_buffer, 
                       span<const copy_region> regions,
                       cuda_device_queue &queue );

    std::shared_ptr<host_buffer> 
    transfer(const std::shared_ptr<device_buffer> &buffer, 
             host_memory_allocator &allocator,
             std::size_t alignment,
             device_queue &queue ) override;

    std::shared_ptr<const host_buffer> 
    transfer(const std::shared_ptr<const device_buffer> &buffer, 
             host_memory_allocator &allocator,
             std::size_t alignment,
             device_queue &queue ) override;

    std::shared_ptr<host_buffer> 
    transfer(const device_buffer *buffer, 
             host_memory_allocator &allocator,
             std::size_t alignment,
             device_queue &queue );

    std::shared_ptr<host_buffer> 
    transfer(const cuda_device_buffer &buffer, 
             cuda_host_memory_allocator &allocator,
             std::size_t alignment,
             cuda_device_queue &queue );

}; 

} // namespace compute
} // namespace xmipp4
