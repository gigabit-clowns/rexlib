// SPDX-License-Identifier: GPL-3.0-only

// Based on: https://leimao.github.io/blog/Proper-CUDA-Error-Checking/

#include "error.hpp"

#include <cstdio>
#include <sstream>

namespace xmipp4
{
namespace cuda
{

static std::string format_error(
	cudaError_t code,
	const char* call,
	const char* file,
	int line
)
{
	std::ostringstream oss;
	oss << "CUDA Runtime Error at: " << file << ":" << line << std::endl;
	oss << cudaGetErrorString(code) << " " << call << std::endl;
	return oss.str();
}

void check(
	cudaError_t code,
	const char* call,
	const char* file,
	int line
)
{
	if (code != cudaSuccess)
	{
		throw error(format_error(code, call, file, line));
	}
}

void check_no_throw(
	cudaError_t code,
	const char* call,
	const char* file,
	int line
) noexcept
{
	if (code == cudaSuccess)
	{
		return;
	}

	// Reporting through the standard streams would allocate, and this runs
	// where there is no way left to handle a failure to do so.
	std::fprintf(
		stderr,
		"CUDA Runtime Error at: %s:%d\n%s %s\n",
		file, line, cudaGetErrorString(code), call
	);
}

} // namespace cuda
} // namespace xmipp4
