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
 * @file cuda_device_queue.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_device_queue.hpp
 * @date 2024-11-27
 * 
 */

#include <xmipp4/cuda/compute/cuda_device_queue_pool.hpp>

#include <xmipp4/cuda/compute/cuda_error.hpp>
#include <xmipp4/cuda/compute/cuda_device.hpp>

#include <cuda_runtime.h>

namespace xmipp4
{
namespace compute
{

cuda_device_queue_pool::cuda_device_queue_pool(int device_index, std::size_t count)
{
    XMIPP4_CUDA_CHECK( cudaSetDevice(device_index) );
    m_queues.resize(count);
}

std::size_t cuda_device_queue_pool::get_size() const noexcept
{
    return m_queues.size();
}

cuda_device_queue& cuda_device_queue_pool::get_queue(std::size_t index)
{
    return m_queues.at(index);
}

} // namespace compute
} // namespace xmipp4
