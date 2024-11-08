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
 * @file cuda_device_to_host_transfer.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines the compute::cuda_device_to_host_transfer class
 * @date 2024-11-06
 * 
 */

#include <xmipp4/core/compute/device_to_host_transfer.hpp>

#include "cuda_device_event.hpp"

namespace xmipp4 
{
namespace compute
{


/**
 * @brief CUDA implementation of the device to host transfer engine.
 * 
 */
class cuda_device_to_host_transfer final
    : public device_to_host_transfer
{
public:
    void transfer_copy(const device_buffer &src_buffer, 
                       const std::shared_ptr<host_buffer> &dst_buffer, 
                       device_queue &queue ) final;

    std::shared_ptr<host_buffer> 
    transfer(const std::shared_ptr<device_buffer> &buffer, 
             host_memory_allocator &allocator,
             device_queue &queue ) final;

    std::shared_ptr<const host_buffer> 
    transfer(const std::shared_ptr<const device_buffer> &buffer, 
             host_memory_allocator &allocator,
             device_queue &queue ) final;


    void wait() final;

private:
    cuda_device_event m_event;
    std::shared_ptr<const host_buffer> m_current;

}; 

} // namespace compute
} // namespace xmipp4
