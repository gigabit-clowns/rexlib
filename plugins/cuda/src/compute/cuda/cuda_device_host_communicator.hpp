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
 * @file cuda_device_host_communicator.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines the compute::cuda_device_host_communicator class
 * @date 2024-11-06
 * 
 */

#include <xmipp4/core/compute/device_host_communicator.hpp>

namespace xmipp4 
{
namespace compute
{


/**
 * @brief CUDA implementation of the host to device transfer engine.
 * 
 */
class cuda_device_host_communicator final
    : public device_host_communicator
{
public:
    void host_to_device(const host_buffer &src_buffer, 
                        device_buffer &dst_buffer, 
                        device_queue &queue ) const final;

    std::shared_ptr<device_buffer> 
    host_to_device_nocopy(const std::shared_ptr<host_buffer> &buffer, 
                          device_memory_allocator &allocator,
                          device_queue &queue ) const final;

    std::shared_ptr<const device_buffer> 
    host_to_device_nocopy(const std::shared_ptr<const host_buffer> &buffer, 
                          device_memory_allocator &allocator,
                          device_queue &queue ) const final;

    void device_to_host(const device_buffer &src_buffer, 
                        host_buffer &dst_buffer, 
                        device_queue &queue ) const final;

    std::shared_ptr<host_buffer> 
    device_to_host_nocopy(const std::shared_ptr<device_buffer> &buffer, 
                          host_memory_allocator &allocator,
                          device_queue &queue ) const final;

    std::shared_ptr<const host_buffer> 
    device_to_host_nocopy(const std::shared_ptr<const device_buffer> &buffer, 
                          host_memory_allocator &allocator,
                          device_queue &queue ) const final;

}; 

} // namespace compute
} // namespace xmipp4
