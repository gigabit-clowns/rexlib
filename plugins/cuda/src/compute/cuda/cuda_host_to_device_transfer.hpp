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

#include "cuda_device_event.hpp"

namespace xmipp4 
{
namespace compute
{


/**
 * @brief CUDA implementation of the host to device transfer engine.
 * 
 */
class cuda_host_to_device_transfer final
    : public host_to_device_transfer
{
public:
    void transfer(const std::shared_ptr<const host_buffer> &src_buffer, 
                  device_buffer &dst_buffer, 
                  device_queue &queue ) final;

    std::shared_ptr<device_buffer> 
    transfer_nocopy(const std::shared_ptr<host_buffer> &buffer, 
                    device_memory_allocator &allocator,
                    device_queue &queue ) final;

    std::shared_ptr<const device_buffer> 
    transfer_nocopy(const std::shared_ptr<const host_buffer> &buffer, 
                    device_memory_allocator &allocator,
                    device_queue &queue ) final;


    void wait() final;
    void wait(device_queue &queue) final;

private:
    cuda_device_event m_event;
    std::shared_ptr<const host_buffer> m_current;

}; 

} // namespace compute
} // namespace xmipp4
