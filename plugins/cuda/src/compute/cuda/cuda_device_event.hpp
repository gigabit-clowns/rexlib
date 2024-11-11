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
 * @file cuda_device_event.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines cuda_device_event class.
 * @date 2024-11-07
 * 
 */

#include <cuda_runtime.h>

namespace xmipp4 
{
namespace compute
{

class device_queue;
class cuda_device_queue;



class cuda_device_event
{
public:
    using handle = cudaEvent_t;

    cuda_device_event();
    cuda_device_event(const cuda_device_event &other) = delete;
    cuda_device_event(cuda_device_event &&other) noexcept;
    ~cuda_device_event();

    cuda_device_event& operator=(const cuda_device_event &other) = delete;
    cuda_device_event& operator=(cuda_device_event &&other) noexcept;

    void swap(cuda_device_event &other) noexcept;
    void reset() noexcept;
    handle get_handle() noexcept;

    void record(cuda_device_queue &queue);
    void record(device_queue &queue);

    void wait(cuda_device_queue &queue) const;
    void wait(device_queue &queue) const;

    void synchronize() const;
    bool query() const;

private:
    handle m_event;

}; 

} // namespace compute
} // namespace xmipp4
