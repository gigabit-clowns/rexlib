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
 * @date 2024-10-30
 * 
 */

#include "cuda_device_queue.hpp"

#include "cuda_error.hpp"
#include "cuda_device.hpp"

#include <utility>
#include <functional>

namespace xmipp4
{
namespace compute
{

cuda_device_queue::cuda_device_queue(cuda_device &device)
{
    XMIPP4_CUDA_CHECK( cudaSetDevice(device.get_index()) );
    XMIPP4_CUDA_CHECK( cudaStreamCreate(&m_stream) );
}

cuda_device_queue::cuda_device_queue(cuda_device_queue &&other) noexcept
    : m_stream(other.m_stream)
{
    other.m_stream = nullptr;
}

cuda_device_queue::~cuda_device_queue()
{
    reset();
}

cuda_device_queue& 
cuda_device_queue::operator=(cuda_device_queue &&other) noexcept
{
    swap(other);
    other.reset();
    return *this;
}

void cuda_device_queue::swap(cuda_device_queue &other) noexcept
{
    std::swap(m_stream, other.m_stream);
}

void cuda_device_queue::reset() noexcept
{
    if (m_stream)
    {
        XMIPP4_CUDA_CHECK( cudaStreamDestroy(m_stream) );
    }
}


cuda_device_queue::handle cuda_device_queue::get_handle() noexcept
{
    return m_stream;
}

void cuda_device_queue::synchronize() const
{
    XMIPP4_CUDA_CHECK( cudaStreamSynchronize(m_stream) );
}

std::size_t cuda_device_queue::get_id() const noexcept
{
    return std::hash<cudaStream_t>()(m_stream);
}

} // namespace compute
} // namespace xmipp4
