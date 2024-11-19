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
 * @file cuda_device_buffer.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines the compute::cuda_device_buffer interface
 * @date 2024-10-30
 * 
 */

#include <xmipp4/core/compute/device_buffer.hpp>

namespace xmipp4 
{
namespace compute
{

class cuda_device_buffer
    : public device_buffer
{
public:
    cuda_device_buffer() = default;
    cuda_device_buffer(const cuda_device_buffer &other) = default;
    cuda_device_buffer(cuda_device_buffer &&other) = default;
    virtual ~cuda_device_buffer() = default;

    cuda_device_buffer& operator=(const cuda_device_buffer &other) = default;
    cuda_device_buffer& operator=(cuda_device_buffer &&other) = default;

    virtual void* get_data() noexcept = 0;
    virtual const void* get_data() const noexcept = 0;

}; 

} // namespace compute
} // namespace xmipp4
