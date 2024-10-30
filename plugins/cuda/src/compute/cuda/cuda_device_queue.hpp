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
 * @brief Defines cuda_device_queue class.
 * @date 2024-10-30
 * 
 */

#include <xmipp4/core/compute/device_queue.hpp>

#include <cuda_runtime.h>

namespace xmipp4 
{
namespace compute
{

class cuda_device_queue_backend;

class cuda_device_queue final
    : public device_queue
{
public:
    cuda_device_queue(int device);
    cuda_device_queue(const cuda_device_queue &other) = delete;
    cuda_device_queue(cuda_device_queue &&other) noexcept;
    virtual ~cuda_device_queue();

    cuda_device_queue& operator=(const cuda_device_queue &other) = delete;
    cuda_device_queue& operator=(cuda_device_queue &&other) noexcept;

    void swap(cuda_device_queue &other) noexcept;
    void reset() noexcept;

    void synchronize() const final;

private:
    cudaStream_t m_stream;

}; 

} // namespace compute
} // namespace xmipp4
