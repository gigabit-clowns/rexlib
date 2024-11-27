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
 * @file cuda_device_queue.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines cuda_device_queue interface
 * @date 2024-11-27
 * 
 */

#include <xmipp4/core/compute/device_queue_pool.hpp>

#include "cuda_device_queue.hpp"

#include <vector>

namespace xmipp4 
{
namespace compute
{

/**
 * @brief Implementation of the device_queue_pool interface to be 
 * able to obtain cuda_device_queue-s.
 * 
 */
class cuda_device_queue_pool final
    : public device_queue_pool
{
public:
    cuda_device_queue_pool(int device_index, std::size_t count);
    cuda_device_queue_pool(const cuda_device_queue_pool &other) = delete;
    cuda_device_queue_pool(cuda_device_queue_pool &&other) = default;
    ~cuda_device_queue_pool() override = default;

    cuda_device_queue_pool&
    operator=(const cuda_device_queue_pool &other) = delete;
    cuda_device_queue_pool&
    operator=(cuda_device_queue_pool &&other) = default;

    std::size_t get_size() const noexcept override;
    cuda_device_queue& get_queue(std::size_t index) override;

private:
    std::vector<cuda_device_queue> m_queues;

}; 

} // namespace compute
} // namespace xmipp4
