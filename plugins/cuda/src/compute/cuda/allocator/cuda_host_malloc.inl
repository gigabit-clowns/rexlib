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
 * @file cuda_host_malloc.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_host_malloc.hpp
 * @date 2024-11-06
 * 
 */

#include "../cuda_error.hpp"
#include "cuda_host_malloc.hpp"

#include <cuda_runtime.h>

namespace xmipp4
{
namespace compute
{

inline
void* cuda_host_malloc::allocate(std::size_t size)
{
    void* result;
    XMIPP4_CUDA_CHECK( cudaMallocHost(&result, size) );
    return result;
}

inline
void cuda_host_malloc::deallocate(void* data, std::size_t)
{
    XMIPP4_CUDA_CHECK( cudaFreeHost(data) );
}

} // namespace compute
} // namespace xmipp4
