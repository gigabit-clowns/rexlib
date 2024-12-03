#pragma once

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
 * @file cuda_host_to_device_transfer.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines the compute::cuda_host_to_device_transfer class
 * @date 2024-11-06
 * 
 */

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

    void transfer_copy_impl(const host_buffer &src_buffer, 
                            cuda_device_buffer &dst_buffer, 
                            cuda_device_queue &queue );

    void transfer_copy(const host_buffer &src_buffer, 
                       device_buffer &dst_buffer, 
                       span<const copy_region> regions,
                       device_queue &queue ) override;

    void transfer_copy_impl(const host_buffer &src_buffer, 
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
    transfer_impl(const host_buffer &buffer, 
                  cuda_device_memory_allocator &allocator,
                  std::size_t alignment,
                  cuda_device_queue &queue );

}; 

} // namespace compute
} // namespace xmipp4
