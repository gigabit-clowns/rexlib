// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <cstddef>

/**
 * @brief Wrapper around cudaHostMalloc and cudaHostFree
 * 
 */
namespace xmipp4 
{
namespace compute
{

struct cuda_host_malloc
{
    /**
     * @brief Allocate memory in the host.
     * 
     * Allocated memory will be managed by CUDA and thus it
     * will be pinned. This means that transfers from/to 
     * devices occur in an efficient manner
     * 
     * @param size Number of bytes to be allocated.
     * @return void* Allocated data.
     */
    static void* allocate(std::size_t size);

    /**
     * @brief Release data.
     * 
     * @param data Data to be released. Must have been obtained from a
     * call to allocate.
     * @param size Number of bytes to be released. Must be equal to 
     * the bytes used when calling allocate.
     * 
     */
    static void deallocate(void* data, std::size_t size);

};

} // namespace compute
} // namespace xmipp4

#include "cuda_host_malloc.inl"
