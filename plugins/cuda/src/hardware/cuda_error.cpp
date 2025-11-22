// SPDX-License-Identifier: GPL-3.0-only

// Based on: https://leimao.github.io/blog/Proper-CUDA-Error-Checking/

#include <xmipp4/cuda/hardware/cuda_error.hpp>

#include <sstream>

namespace xmipp4
{
namespace hardware
{

void cuda_check(
	cudaError_t code, 
	const char* call, 
	const char* file,
	int line
)
{
	if (code != cudaSuccess)
	{
		std::ostringstream oss;
		oss << "CUDA Runtime Error at: " << file << ":" << line << std::endl;
		oss << cudaGetErrorString(code) << " " << call << std::endl;
		throw cuda_error(oss.str());
	}
}

} // namespace hardware
} // namespace xmipp4
