// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/cuda/compute/cuda_device_copy.hpp>

#include <xmipp4/cuda/compute/cuda_device_queue.hpp>
#include <xmipp4/cuda/compute/cuda_device_buffer.hpp>

#include "cuda_buffer_memcpy.hpp"

namespace xmipp4
{
namespace compute
{

void cuda_device_copy::copy(const device_buffer &src_buffer,
                            device_buffer &dst_buffer, 
                            device_queue &queue ) 
{
    copy(
        dynamic_cast<const cuda_device_buffer&>(src_buffer),
        dynamic_cast<cuda_device_buffer&>(dst_buffer),
        dynamic_cast<cuda_device_queue&>(queue)
    );
}

void cuda_device_copy::copy(const cuda_device_buffer &src_buffer, 
                            cuda_device_buffer &dst_buffer, 
                            cuda_device_queue &queue )
{
    cuda_memcpy(src_buffer, dst_buffer, queue);
}

void cuda_device_copy::copy(const device_buffer &src_buffer,
                            device_buffer &dst_buffer, 
                            span<const copy_region> regions,
                            device_queue &queue ) 
{
    copy(
        dynamic_cast<const cuda_device_buffer&>(src_buffer),
        dynamic_cast<cuda_device_buffer&>(dst_buffer),
        regions,
        dynamic_cast<cuda_device_queue&>(queue)
    );
}

void cuda_device_copy::copy(const cuda_device_buffer &src_buffer,
                            cuda_device_buffer &dst_buffer,
                            span<const copy_region> regions,
                            cuda_device_queue &queue )
{
    cuda_memcpy(src_buffer, dst_buffer, regions, queue);
}

} // namespace compute
} // namespace xmipp4
