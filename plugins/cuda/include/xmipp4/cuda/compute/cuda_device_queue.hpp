// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <xmipp4/core/compute/device_queue.hpp>

#include <cuda_runtime.h>

namespace xmipp4 
{
namespace compute
{

class cuda_device;

class cuda_device_queue final
    : public device_queue
{
public:
    using handle = cudaStream_t;

    cuda_device_queue();
    explicit cuda_device_queue(cuda_device &device);
    cuda_device_queue(const cuda_device_queue &other) = delete;
    cuda_device_queue(cuda_device_queue &&other) noexcept;
    ~cuda_device_queue() override;

    cuda_device_queue& operator=(const cuda_device_queue &other) = delete;
    cuda_device_queue& operator=(cuda_device_queue &&other) noexcept;

    void swap(cuda_device_queue &other) noexcept;
    void reset() noexcept;
    handle get_handle() noexcept;

    void wait_until_completed() const override;
    bool is_idle() const noexcept override;

private:
    handle m_stream;

}; 

} // namespace compute
} // namespace xmipp4
