// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <xmipp4/core/compute/device_event.hpp>
#include <xmipp4/core/compute/device_to_host_event.hpp>

#include <cuda_runtime.h>

namespace xmipp4 
{
namespace compute
{

class device_queue;
class cuda_device_queue;



class cuda_event final
    : public device_event
    , public device_to_host_event
{
public:
    using handle = cudaEvent_t;

    cuda_event();
    cuda_event(const cuda_event &other) = delete;
    cuda_event(cuda_event &&other) noexcept;
    ~cuda_event() override;

    cuda_event& operator=(const cuda_event &other) = delete;
    cuda_event& operator=(cuda_event &&other) noexcept;

    void swap(cuda_event &other) noexcept;
    void reset() noexcept;
    handle get_handle() noexcept;

    void signal(device_queue &queue) override;
    void signal(cuda_device_queue &queue);

    void wait() const override;
    void wait(device_queue &queue) const override;
    void wait(cuda_device_queue &queue) const;

    bool is_signaled() const override;

private:
    handle m_event;

}; 

} // namespace compute
} // namespace xmipp4
