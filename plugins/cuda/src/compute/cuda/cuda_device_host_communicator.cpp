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
 * @file cuda_device_host_communicator.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_device_host_communicator.hpp
 * @date 2024-11-06
 * 
 */

#include "cuda_device_host_communicator.hpp"

#include "cuda_device_queue.hpp"
#include "cuda_device_buffer.hpp"
#include "cuda_device_memory_allocator.hpp"
#include "cuda_host_memory_allocator.hpp"

#include <xmipp4/core/compute/host_buffer.hpp>

#include <cuda_runtime.h>

namespace xmipp4
{
namespace compute
{

void cuda_device_host_communicator::host_to_device(const host_buffer &src_buffer, 
                                                   device_buffer &dst_buffer, 
                                                   device_queue &queue ) const
{
    if (src_buffer.get_type() != dst_buffer.get_type())
    {
        throw std::invalid_argument("Both buffers must have the same numerical type");
    }
    
    if (src_buffer.get_count() != dst_buffer.get_count())
    {
        throw std::invalid_argument("Both buffers must have the same element count");
    }

    // TODO check return
    const auto element_size = get_size(src_buffer.get_type());
    cudaMemcpyAsync(
        dynamic_cast<cuda_device_buffer&>(dst_buffer).get_data(),
        src_buffer.get_data(),
        element_size*src_buffer.get_count(),
        cudaMemcpyHostToDevice,
        dynamic_cast<cuda_device_queue&>(queue).get_handle()
    );
}

std::shared_ptr<device_buffer> 
cuda_device_host_communicator::host_to_device_nocopy(const std::shared_ptr<host_buffer> &buffer, 
                                                     device_memory_allocator &allocator,
                                                     device_queue &queue ) const
{
    const auto result = allocator.create_buffer_shared(
        buffer->get_type(), buffer->get_count(), queue
    );

    host_to_device(*buffer, *result, queue);

    return result;
}

std::shared_ptr<const device_buffer> 
cuda_device_host_communicator::host_to_device_nocopy(const std::shared_ptr<const host_buffer> &buffer, 
                                                     device_memory_allocator &allocator,
                                                     device_queue &queue ) const
{
    const auto result = allocator.create_buffer_shared(
        buffer->get_type(), buffer->get_count(), queue
    );

    host_to_device(*buffer, *result, queue);

    return result;
}

void cuda_device_host_communicator::device_to_host(const device_buffer &src_buffer, 
                                                   host_buffer &dst_buffer, 
                                                   device_queue &queue ) const
{
    if (src_buffer.get_type() != dst_buffer.get_type())
    {
        throw std::invalid_argument("Both buffers must have the same numerical type");
    }
    
    if (src_buffer.get_count() != dst_buffer.get_count())
    {
        throw std::invalid_argument("Both buffers must have the same element count");
    }

    // TODO check return
    const auto element_size = get_size(src_buffer.get_type());
    cudaMemcpyAsync(
        dst_buffer.get_data(),
        dynamic_cast<const cuda_device_buffer&>(src_buffer).get_data(),
        element_size*src_buffer.get_count(),
        cudaMemcpyDeviceToHost,
        dynamic_cast<cuda_device_queue&>(queue).get_handle()
    );
}

std::shared_ptr<host_buffer> 
cuda_device_host_communicator::device_to_host_nocopy(const std::shared_ptr<device_buffer> &buffer, 
                                                     host_memory_allocator &allocator,
                                                     device_queue &queue ) const
{
    const auto result = allocator.create_buffer_shared(
        buffer->get_type(), buffer->get_count()
    );

    device_to_host(*buffer, *result, queue);

    return result;
}

std::shared_ptr<const host_buffer> 
cuda_device_host_communicator::device_to_host_nocopy(const std::shared_ptr<const device_buffer> &buffer, 
                                                     host_memory_allocator &allocator,
                                                     device_queue &queue ) const
{
    const auto result = allocator.create_buffer_shared(
        buffer->get_type(), buffer->get_count()
    );

    device_to_host(*buffer, *result, queue);

    return result;
}

} // namespace compute
} // namespace xmipp4
