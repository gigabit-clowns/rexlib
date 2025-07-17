// SPDX-License-Identifier: GPL-3.0-only

#include "../cuda_error.hpp"
#include "cuda_device_malloc.hpp"

#include <cuda_runtime.h>

namespace xmipp4
{
namespace compute
{

inline
cuda_device_malloc::cuda_device_malloc(int device_id) noexcept
    : m_device_id(device_id)
{   
}

inline
void* cuda_device_malloc::allocate(std::size_t size) const
{
    void* result;
    XMIPP4_CUDA_CHECK( cudaSetDevice(m_device_id) );
    XMIPP4_CUDA_CHECK( cudaMalloc(&result, size) );
    return result;
}

inline
void cuda_device_malloc::deallocate(void* data, std::size_t) const
{
    XMIPP4_CUDA_CHECK( cudaFree(data) );
}

} // namespace compute
} // namespace xmipp4
