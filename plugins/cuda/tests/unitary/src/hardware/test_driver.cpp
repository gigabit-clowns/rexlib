// SPDX-License-Identifier: GPL-3.0-only

#include "test_driver.hpp"

#include <cuda_runtime.h>

namespace rexlib
{
namespace cuda
{

bool has_test_device() noexcept
{
	int count = 0;
	return cudaGetDeviceCount(&count) == cudaSuccess && count > 0;
}

} // namespace cuda
} // namespace rexlib
