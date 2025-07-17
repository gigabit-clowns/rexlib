// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <xmipp4/core/compute/device_buffer.hpp>

namespace xmipp4 
{
namespace compute
{

class cuda_device_buffer
    : public device_buffer
{
public:
    cuda_device_buffer() = default;
    cuda_device_buffer(const cuda_device_buffer &other) = default;
    cuda_device_buffer(cuda_device_buffer &&other) = default;
    virtual ~cuda_device_buffer() = default;

    cuda_device_buffer& operator=(const cuda_device_buffer &other) = default;
    cuda_device_buffer& operator=(cuda_device_buffer &&other) = default;

    virtual void* get_data() noexcept = 0;
    virtual const void* get_data() const noexcept = 0;

}; 

} // namespace compute
} // namespace xmipp4
