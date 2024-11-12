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
 * @file cuda_device_to_host_transfer.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_device_to_host_transfer.hpp
 * @date 2024-11-06
 * 
 */

#include "cuda_device_to_host_transfer.hpp"

#include "cuda_device_queue.hpp"
#include "cuda_device_buffer.hpp"
#include "cuda_host_memory_allocator.hpp"

#include <xmipp4/core/memory/align.hpp>
#include <xmipp4/core/compute/host_buffer.hpp>
#include <xmipp4/core/compute/checks.hpp>


namespace xmipp4
{
namespace compute
{

static void require_nonnull_dst(const std::shared_ptr<host_buffer> &buf)
{
    if (!buf)
    {
        throw std::invalid_argument("dst_buffer cannot be nullptr");
    }
}

void cuda_device_to_host_transfer::transfer_copy(const device_buffer &src_buffer, 
                                                 const std::shared_ptr<host_buffer> &dst_buffer, 
                                                 device_queue &queue )
{
    require_nonnull_dst(dst_buffer);

    const auto &cuda_src_buffer = 
        dynamic_cast<const cuda_device_buffer&>(src_buffer);    
    auto &cuda_queue = dynamic_cast<cuda_device_queue&>(queue);
    const auto type = require_same_type(
        src_buffer.get_type(), dst_buffer->get_type()
    );
    const auto count = require_same_count(
        src_buffer.get_count(), dst_buffer->get_count()
    );
    const auto element_size = get_size(type);

    // TODO check return
    cudaMemcpyAsync(
        dst_buffer->get_data(),
        cuda_src_buffer.get_data(),
        element_size*count,
        cudaMemcpyDeviceToHost,
        cuda_queue.get_handle()
    );

    update_current(dst_buffer, cuda_queue);
}

void cuda_device_to_host_transfer::transfer_copy(const device_buffer &src_buffer,
                                                 const std::shared_ptr<host_buffer> &dst_buffer,
                                                 span<const copy_region> regions,
                                                 device_queue &queue )
{
    require_nonnull_dst(dst_buffer);

    auto &cuda_queue = dynamic_cast<cuda_device_queue&>(queue);
    const auto* src_data = 
        dynamic_cast<const cuda_device_buffer&>(src_buffer).get_data();
    auto* dst_data = dst_buffer->get_data();
    const auto src_count = src_buffer.get_count();
    const auto dst_count = dst_buffer->get_count();
    const auto type = require_same_type(
        src_buffer.get_type(), dst_buffer->get_type()
    );
    const auto element_size = get_size(type);

    for (const copy_region &region : regions)
    {
        require_valid_region(region, src_count, dst_count);

        // TODO check return
        const auto region_bytes = as_bytes(region, element_size);
        cudaMemcpyAsync(
            memory::offset_bytes(dst_data, region_bytes.get_destination_offset()),
            memory::offset_bytes(src_data, region_bytes.get_source_offset()),
            region_bytes.get_count(),
            cudaMemcpyDeviceToHost,
            cuda_queue.get_handle()
        );
    }

    update_current(dst_buffer, cuda_queue);
}

std::shared_ptr<host_buffer> 
cuda_device_to_host_transfer::transfer(const std::shared_ptr<device_buffer> &buffer, 
                                       host_memory_allocator &allocator,
                                       device_queue &queue )
{
    std::shared_ptr<host_buffer> result;

    if (buffer)
    {
        result = allocator.create_buffer_shared(
            buffer->get_type(), buffer->get_count()
        );

        transfer_copy(*buffer, result, queue);
    }

    return result;
}

std::shared_ptr<const host_buffer> 
cuda_device_to_host_transfer::transfer(const std::shared_ptr<const device_buffer> &buffer, 
                                       host_memory_allocator &allocator,
                                       device_queue &queue )
{
    std::shared_ptr<const host_buffer> result;

    if (buffer)
    {
        auto tmp = allocator.create_buffer_shared(
            buffer->get_type(), buffer->get_count()
        );

        transfer_copy(*buffer, tmp, queue);
        result = std::move(tmp);
    }

    return result;
}

void cuda_device_to_host_transfer::wait()
{
    if (m_current)
    {
        m_event.synchronize();
        m_current = nullptr;
    }
}

void cuda_device_to_host_transfer::update_current(std::shared_ptr<const host_buffer> buffer, 
                                                  cuda_device_queue &queue )
{
    wait(); // Wait the previous transfer to complete
    m_current = std::move(buffer);
    m_event.record(queue);

}

} // namespace compute
} // namespace xmipp4
