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
 * @file cuda_error.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines a cuda error exception and methods that throw it.
 * @date 2024-11-17
 * 
 */

#include <stdexcept>

#include <cuda_runtime.h>

namespace xmipp4 
{
namespace compute
{

/**
 * @brief Exception class representing a CUDA runtime error.
 * 
 */
class cuda_error
    : public std::runtime_error
{
    using runtime_error::runtime_error;
};

/**
 * @brief Check CUDA return code and throw an exception on failure.
 * 
 * @param code CUDA return code
 * @param call String identifying the CUDA function call.
 * @param file File where the error ocurred.
 * @param line Line where the error ocurred.
 * 
 */
void cuda_check(cudaError_t code, 
                const char* call, 
                const char* file,
                int line );

/**
 * @brief Calls cuda_check filling the call name, filename and line number.
 * 
 */
#define XMIPP4_CUDA_CHECK(val) cuda_check((val), #val, __FILE__, __LINE__)

} // namespace compute
} // namespace xmipp4
