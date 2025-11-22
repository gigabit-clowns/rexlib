// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <stdexcept>

#include "../dynamic_shared_object.h"

#include <cuda_runtime.h>

namespace xmipp4 
{
namespace hardware
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
 * @param file File where the error occurred.
 * @param line Line where the error occurred.
 * 
 */
XMIPP4_HARDWARE_CUDA_API
void cuda_check(
	cudaError_t code, 
	const char* call, 
	const char* file,
	int line
);

/**
 * @brief Calls cuda_check filling the call name, filename and line number.
 * 
 */
#define XMIPP4_CUDA_CHECK(val) cuda_check((val), #val, __FILE__, __LINE__)

} // namespace hardware
} // namespace xmipp4
