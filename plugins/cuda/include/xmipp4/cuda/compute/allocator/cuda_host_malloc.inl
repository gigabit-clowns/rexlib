// SPDX-License-Identifier: GPL-3.0-only

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
