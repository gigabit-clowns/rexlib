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
 * @file cuda_device_malloc.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines cuda_device_malloc class.
 * @date 2024-11-06
 * 
 */

#include <cstddef>

namespace xmipp4 
{
namespace compute
{
 
/**
 * @brief Wrapper around cudaMalloc/cudaFree that targets
 * an specific device
 * 
 */
class cuda_device_malloc
{
public:
    explicit cuda_device_malloc(int device_id) noexcept;
    cuda_device_malloc(const cuda_device_malloc &other) = default;
    cuda_device_malloc(cuda_device_malloc &&other) = default;
    ~cuda_device_malloc() = default;

    cuda_device_malloc& operator=(const cuda_device_malloc &other) = default;
    cuda_device_malloc& operator=(cuda_device_malloc &&other) = default;

    /**
     * @brief Allocate memory in the targeted device.
     * 
     * @param size Number of bytes to be allocated.
     * @return void* Allocated data.
     */
    void* allocate(std::size_t size) const;

    /**
     * @brief Release data.
     * 
     * @param data Data to be released. Must have been obtained from a
     * call to allocate.
     * @param size Number of bytes to be released. Must be equal to 
     * the bytes used when calling allocate.
     * 
     */
    void deallocate(void* data, std::size_t size) const;

private:
    int m_device_id;

};

} // namespace compute
} // namespace xmipp4

#include "cuda_device_malloc.inl"
