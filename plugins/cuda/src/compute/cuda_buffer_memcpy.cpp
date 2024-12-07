/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

/**
 * @file cuda_buffer_memcpy.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_buffer_memcpy.hpp
 * @date 2024-12-07
 * 
 */

#include "cuda_buffer_memcpy.hpp"

#include <xmipp4/cuda/compute/cuda_error.hpp>
#include <xmipp4/cuda/compute/cuda_device_queue.hpp>
#include <xmipp4/cuda/compute/cuda_device_buffer.hpp>

#include <xmipp4/core/memory/align.hpp>
#include <xmipp4/core/compute/checks.hpp>
#include <xmipp4/core/compute/host_buffer.hpp>

#include <cuda_runtime.h>

namespace xmipp4
{
namespace compute
{

template <cudaMemcpyKind direction, typename SrcBuffer, typename DstBuffer>
static void cuda_memcpy_impl(const SrcBuffer &src_buffer,
                             DstBuffer &dst_buffer,
                             cuda_device_queue &queue )
{
    const auto count = require_same_buffer_size(
        src_buffer.get_size(), dst_buffer.get_size()
    );

    XMIPP4_CUDA_CHECK(
        cudaMemcpyAsync(
            dst_buffer.get_data(),
            src_buffer.get_data(),
            count,
            direction,
            queue.get_handle()
        )
    );
}

template <cudaMemcpyKind direction, typename SrcBuffer, typename DstBuffer>
static void cuda_memcpy_impl(const SrcBuffer &src_buffer,
                             DstBuffer &dst_buffer,
                             span<const copy_region> regions,
                             cuda_device_queue &queue )
{
    const auto *src_data = src_buffer.get_data();
    auto *dst_data = dst_buffer.get_data();
    const auto src_count = src_buffer.get_size();
    const auto dst_count = dst_buffer.get_size();

    for (const copy_region &region : regions)
    {
        require_valid_region(region, src_count, dst_count);

        XMIPP4_CUDA_CHECK(
            cudaMemcpyAsync(
                memory::offset_bytes(dst_data, region.get_destination_offset()),
                memory::offset_bytes(src_data, region.get_source_offset()),
                region.get_count(),
                direction,
                queue.get_handle()
            )
        );
    }
}



void cuda_memcpy(const cuda_device_buffer &src, 
                 cuda_device_buffer &dst,
                 cuda_device_queue &queue )
{
    cuda_memcpy_impl<cudaMemcpyDeviceToDevice>(src, dst, queue);
}

void cuda_memcpy(const cuda_device_buffer &src, 
                 cuda_device_buffer &dst,
                 span<const copy_region> regions,
                 cuda_device_queue &queue )
{
    cuda_memcpy_impl<cudaMemcpyDeviceToDevice>(src, dst, regions, queue);
}

void cuda_memcpy(const cuda_device_buffer &src, 
                 host_buffer &dst,
                 cuda_device_queue &queue )
{
    cuda_memcpy_impl<cudaMemcpyDeviceToHost>(src, dst, queue);
}

void cuda_memcpy(const cuda_device_buffer &src, 
                 host_buffer &dst,
                 span<const copy_region> regions,
                 cuda_device_queue &queue )
{
    cuda_memcpy_impl<cudaMemcpyDeviceToHost>(src, dst, regions, queue);
}

void cuda_memcpy(const host_buffer &src, 
                 cuda_device_buffer &dst,
                 cuda_device_queue &queue )
{
    cuda_memcpy_impl<cudaMemcpyHostToDevice>(src, dst, queue);
}

void cuda_memcpy(const host_buffer &src, 
                 cuda_device_buffer &dst,
                 span<const copy_region> regions,
                 cuda_device_queue &queue )
{
    cuda_memcpy_impl<cudaMemcpyHostToDevice>(src, dst, regions, queue);
}

} // namespace compute
} // namespace xmipp4
