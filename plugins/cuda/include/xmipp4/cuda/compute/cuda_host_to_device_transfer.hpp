// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <xmipp4/core/compute/host_to_device_transfer.hpp>

#include "cuda_event.hpp"

namespace xmipp4 
{
namespace compute
{

class cuda_device_buffer;
class cuda_device_queue;
class cuda_device_memory_allocator;

/**
 * @brief CUDA implementation of the host to device transfer engine.
 * 
 */
class cuda_host_to_device_transfer final
    : public host_to_device_transfer
{
public:
    void transfer_copy(const host_buffer &src_buffer, 
                       device_buffer &dst_buffer, 
                       device_queue &queue ) override;

    void transfer_copy(const host_buffer &src_buffer, 
                       cuda_device_buffer &dst_buffer, 
                       cuda_device_queue &queue );

    void transfer_copy(const host_buffer &src_buffer, 
                       device_buffer &dst_buffer, 
                       span<const copy_region> regions,
                       device_queue &queue ) override;

    void transfer_copy(const host_buffer &src_buffer, 
                       cuda_device_buffer &dst_buffer, 
                       span<const copy_region> regions,
                       cuda_device_queue &queue );

    std::shared_ptr<device_buffer> 
    transfer(const std::shared_ptr<host_buffer> &buffer, 
             device_memory_allocator &allocator,
             std::size_t alignment,
             device_queue &queue ) override;

    std::shared_ptr<const device_buffer> 
    transfer(const std::shared_ptr<const host_buffer> &buffer, 
             device_memory_allocator &allocator,
             std::size_t alignment,
             device_queue &queue ) override;

    std::shared_ptr<cuda_device_buffer> 
    transfer(const host_buffer *buffer, 
             device_memory_allocator &allocator,
             std::size_t alignment,
             device_queue &queue );

    std::shared_ptr<cuda_device_buffer> 
    transfer(const host_buffer &buffer, 
             cuda_device_memory_allocator &allocator,
             std::size_t alignment,
             cuda_device_queue &queue );

}; 

} // namespace compute
} // namespace xmipp4
