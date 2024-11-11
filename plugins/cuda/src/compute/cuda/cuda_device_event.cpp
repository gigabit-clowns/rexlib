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
 * @file cuda_device_event.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_device_event.hpp
 * @date 2024-11-07
 * 
 */

#include "cuda_device_event.hpp"

#include "cuda_device_queue.hpp"

#include <utility>

namespace xmipp4
{
namespace compute
{

cuda_device_event::cuda_device_event()
{
    cudaEventCreate(&m_event); // TODO check
}

cuda_device_event::cuda_device_event(cuda_device_event &&other) noexcept
    : m_event(other.m_event)
{
    other.m_event = nullptr;
}

cuda_device_event::~cuda_device_event()
{
    reset();
}

cuda_device_event& 
cuda_device_event::operator=(cuda_device_event &&other) noexcept
{
    swap(other);
    other.reset();
    return *this;
}

void cuda_device_event::swap(cuda_device_event &other) noexcept
{
    std::swap(m_event, other.m_event);
}

void cuda_device_event::reset() noexcept
{
    if (m_event)
    {
        cudaEventDestroy(m_event); // TODO check
    }
}

cuda_device_event::handle cuda_device_event::get_handle() noexcept
{
    return m_event;
}



void cuda_device_event::record(cuda_device_queue &queue)
{
    cudaEventRecord(m_event, queue.get_handle()); // TODO check return
}

void cuda_device_event::record(device_queue &queue)
{
    record(dynamic_cast<cuda_device_queue&>(queue));
}

void cuda_device_event::wait(cuda_device_queue &queue) const
{
    cudaStreamWaitEvent(queue.get_handle(), m_event, cudaEventWaitDefault); // TODO check return
}

void cuda_device_event::wait(device_queue &queue) const
{
    wait(dynamic_cast<cuda_device_queue&>(queue));
}

void cuda_device_event::synchronize() const
{
    cudaEventSynchronize(m_event); // TODO check return
}

bool cuda_device_event::is_signaled() const
{
    return cudaEventQuery(m_event) == cudaSuccess; // TODO check errors
}

} // namespace compute
} // namespace xmipp4
