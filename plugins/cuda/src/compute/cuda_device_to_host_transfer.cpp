// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/cuda/compute/cuda_device_to_host_transfer.hpp>

#include <xmipp4/cuda/compute/cuda_device_queue.hpp>
#include <xmipp4/cuda/compute/cuda_device_buffer.hpp>
#include <xmipp4/cuda/compute/cuda_host_memory_allocator.hpp>

#include "cuda_buffer_memcpy.hpp"

namespace xmipp4
{
namespace compute
{

void cuda_device_to_host_transfer::transfer_copy(const device_buffer &src_buffer, 
                                                 host_buffer &dst_buffer, 
                                                 device_queue &queue )
{
    transfer_copy(
        dynamic_cast<const cuda_device_buffer&>(src_buffer),
        dst_buffer,
        dynamic_cast<cuda_device_queue&>(queue)
    );
}

void cuda_device_to_host_transfer::transfer_copy(const cuda_device_buffer &src_buffer, 
                                                 host_buffer &dst_buffer, 
                                                 cuda_device_queue &queue )
{
    cuda_memcpy(src_buffer, dst_buffer, queue);
}

void cuda_device_to_host_transfer::transfer_copy(const device_buffer &src_buffer,
                                                 host_buffer &dst_buffer,
                                                 span<const copy_region> regions,
                                                 device_queue &queue )
{
    transfer_copy(
        dynamic_cast<const cuda_device_buffer&>(src_buffer),
        dst_buffer,
        regions,
        dynamic_cast<cuda_device_queue&>(queue)
    );
}

void cuda_device_to_host_transfer::transfer_copy(const cuda_device_buffer &src_buffer,
                                                 host_buffer &dst_buffer,
                                                 span<const copy_region> regions,
                                                 cuda_device_queue &queue )
{
    cuda_memcpy(src_buffer, dst_buffer, regions, queue);
}

std::shared_ptr<host_buffer> 
cuda_device_to_host_transfer::transfer(const std::shared_ptr<device_buffer> &buffer, 
                                       host_memory_allocator &allocator,
                                       std::size_t alignment,
                                       device_queue &queue )
{
    return transfer(buffer.get(), allocator, alignment, queue);
}

std::shared_ptr<const host_buffer> 
cuda_device_to_host_transfer::transfer(const std::shared_ptr<const device_buffer> &buffer, 
                                       host_memory_allocator &allocator,
                                       std::size_t alignment,
                                       device_queue &queue )
{
    return transfer(buffer.get(), allocator, alignment, queue);
}

std::shared_ptr<host_buffer> 
cuda_device_to_host_transfer::transfer(const device_buffer *buffer, 
                                       host_memory_allocator &allocator,
                                       std::size_t alignment,
                                       device_queue &queue )
{
    std::shared_ptr<host_buffer> result;

    if (buffer)
    {
        result = transfer(
            dynamic_cast<const cuda_device_buffer&>(*buffer),
            dynamic_cast<cuda_host_memory_allocator&>(allocator),
            alignment,
            dynamic_cast<cuda_device_queue&>(queue)
        );
    }

    return result;
}

std::shared_ptr<host_buffer> 
cuda_device_to_host_transfer::transfer(const cuda_device_buffer &buffer, 
                                       cuda_host_memory_allocator &allocator,
                                       std::size_t alignment,
                                       cuda_device_queue &queue )
{
    std::shared_ptr<host_buffer> result;

    result = allocator.create_host_buffer(
        buffer.get_size(),
        alignment,
        queue
    );

    transfer_copy(buffer, *result, queue);

    return result;
}



} // namespace compute
} // namespace xmipp4
