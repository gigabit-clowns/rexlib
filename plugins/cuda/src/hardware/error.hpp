// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <stdexcept>

#include <cuda_runtime.h>

namespace xmipp4
{
namespace cuda
{

/**
 * @brief Exception class representing a CUDA runtime error.
 *
 */
class error
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
 * @throws error If the code is not cudaSuccess.
 */
void check(
	cudaError_t code,
	const char* call,
	const char* file,
	int line
);

/**
 * @brief Check the return code of an allocation and throw on failure.
 *
 * Running out of memory is reported as @c std::bad_alloc rather than as an
 * @ref error, since it is the one failure callers react to and they recognize
 * it by its type.
 *
 * @param code CUDA return code
 * @param call String identifying the CUDA function call.
 * @param file File where the error occurred.
 * @param line Line where the error occurred.
 *
 * @throws std::bad_alloc If the code reports that memory ran out.
 * @throws error If the code reports any other failure.
 */
void check_allocation(
	cudaError_t code,
	const char* call,
	const char* file,
	int line
);

/**
 * @brief Check CUDA return code, reporting failures instead of throwing.
 *
 * Meant for destructors and other noexcept contexts. Failures are written to
 * the standard error stream.
 *
 * @param code CUDA return code
 * @param call String identifying the CUDA function call.
 * @param file File where the error occurred.
 * @param line Line where the error occurred.
 *
 */
void check_no_throw(
	cudaError_t code,
	const char* call,
	const char* file,
	int line
) noexcept;

/**
 * @brief Calls check filling the call name, filename and line number.
 *
 */
#define XMIPP4_CUDA_CHECK(val) \
	::xmipp4::cuda::check((val), #val, __FILE__, __LINE__)

/**
 * @brief Calls check_allocation filling the call name, filename and line
 * number.
 *
 */
#define XMIPP4_CUDA_CHECK_ALLOCATION(val) \
	::xmipp4::cuda::check_allocation((val), #val, __FILE__, __LINE__)

/**
 * @brief Calls check_no_throw filling the call name, filename and line number.
 *
 */
#define XMIPP4_CUDA_CHECK_NO_THROW(val) \
	::xmipp4::cuda::check_no_throw((val), #val, __FILE__, __LINE__)

} // namespace cuda
} // namespace xmipp4
