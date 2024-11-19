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
 * @file cuda_error.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_error.hpp
 * @date 2024-11-17
 * 
 * Based on:
 * https://leimao.github.io/blog/Proper-CUDA-Error-Checking/
 * 
 */

#include <xmipp4/cuda/compute/cuda_error.hpp>

#include <sstream>

namespace xmipp4
{
namespace compute
{

void cuda_check(cudaError_t code, 
                const char* call, 
                const char* file,
                int line )
{
    if (code != cudaSuccess)
    {
        std::ostringstream oss;
        oss << "CUDA Runtime Error at: " << file << ":" << line << std::endl;
        oss << cudaGetErrorString(code) << " " << call << std::endl;
        throw cuda_error(oss.str());
    }
}

} // namespace compute
} // namespace xmipp4

