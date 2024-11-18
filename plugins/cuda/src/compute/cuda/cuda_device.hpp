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
 * @file cuda_device.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines cuda_device class.
 * @date 2024-10-30
 * 
 */

#include <xmipp4/core/compute/device.hpp>

namespace xmipp4 
{
namespace compute
{

class cuda_device_backend;

class cuda_device final
    : public device
{
public:
    cuda_device(int device);
    cuda_device(const cuda_device &other) = default;
    cuda_device(cuda_device &&other) = default;
    ~cuda_device() override = default;

    cuda_device& operator=(const cuda_device &other) = default;
    cuda_device& operator=(cuda_device &&other) = default;

    std::unique_ptr<device_queue> create_queue() override;
    std::shared_ptr<device_queue> create_queue_shared() override;

    std::unique_ptr<device_memory_allocator>
    create_device_memory_allocator() override;
    std::shared_ptr<device_memory_allocator>
    create_device_memory_allocator_shared() override;

    std::unique_ptr<host_memory_allocator>
    create_host_memory_allocator() override;
    std::shared_ptr<host_memory_allocator>
    create_host_memory_allocator_shared() override;

    std::unique_ptr<host_to_device_transfer> 
    create_host_to_device_transfer() override;
    std::shared_ptr<host_to_device_transfer> 
    create_host_to_device_transfer_shared() override;

    std::unique_ptr<device_to_host_transfer> 
    create_device_to_host_transfer() override;
    std::shared_ptr<device_to_host_transfer> 
    create_device_to_host_transfer_shared() override;

    std::unique_ptr<device_copy> 
    create_device_copy() override;
    std::shared_ptr<device_copy> 
    create_device_copy_shared() override;

    std::unique_ptr<device_event> create_device_event() override;
    std::shared_ptr<device_event> create_device_event_shared() override;

    std::unique_ptr<device_to_host_event> 
    create_device_to_host_event() override;
    std::shared_ptr<device_to_host_event> 
    create_device_to_host_event_shared() override;

private:
    int m_device;

}; 

} // namespace compute
} // namespace xmipp4
