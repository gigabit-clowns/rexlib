// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <cstddef>

namespace xmipp4 
{
namespace compute
{
 
/**
 * @brief Wrapper around cudaMalloc/cudaFree that targets
 * an specific device
 * 
 */
class cuda_device_malloc
{
public:
    explicit cuda_device_malloc(int device_id) noexcept;
    cuda_device_malloc(const cuda_device_malloc &other) = default;
    cuda_device_malloc(cuda_device_malloc &&other) = default;
    ~cuda_device_malloc() = default;

    cuda_device_malloc& operator=(const cuda_device_malloc &other) = default;
    cuda_device_malloc& operator=(cuda_device_malloc &&other) = default;

    /**
     * @brief Allocate memory in the targeted device.
     * 
     * @param size Number of bytes to be allocated.
     * @return void* Allocated data.
     */
    void* allocate(std::size_t size) const;

    /**
     * @brief Release data.
     * 
     * @param data Data to be released. Must have been obtained from a
     * call to allocate.
     * @param size Number of bytes to be released. Must be equal to 
     * the bytes used when calling allocate.
     * 
     */
    void deallocate(void* data, std::size_t size) const;

private:
    int m_device_id;

};

} // namespace compute
} // namespace xmipp4

#include "cuda_device_malloc.inl"
