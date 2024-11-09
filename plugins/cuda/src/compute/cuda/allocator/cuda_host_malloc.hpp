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
 * @file cuda_host_malloc.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines cuda_host_malloc class.
 * @date 2024-11-06
 * 
 */

#include <cstddef>

/**
 * @brief Wrapper around cudaHostMalloc and cudaHostFree
 * 
 */
namespace xmipp4 
{
namespace compute
{

struct cuda_host_malloc
{
    /**
     * @brief Allocate memory in the host.
     * 
     * Allocated memory will be managed by CUDA and thus it
     * will be pinned. This means that transfers from/to 
     * devices occur in an efficient manner
     * 
     * @param size Number of bytes to be allocated.
     * @return void* Allocated data.
     */
    static void* allocate(std::size_t size);

    /**
     * @brief Release data.
     * 
     * @param data Data to be released. Must have been obtained from a
     * call to allocate.
     * @param size Number of bytes to be released. Must be equal to 
     * the bytes used when calling allocate.
     * 
     */
    static void deallocate(void* data, std::size_t size);
};



} // namespace compute
} // namespace xmipp4

#include "cuda_host_malloc.inl"
