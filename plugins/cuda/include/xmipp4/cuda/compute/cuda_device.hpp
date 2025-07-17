// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <xmipp4/core/compute/device.hpp>

#include "cuda_device_queue_pool.hpp"

namespace xmipp4 
{
namespace compute
{

class device_create_parameters;

class cuda_device final
    : public device
{
public:
    cuda_device(int device, const device_create_parameters &params);
    cuda_device(const cuda_device &other) = delete;
    cuda_device(cuda_device &&other) = default;
    ~cuda_device() override = default;

    cuda_device& operator=(const cuda_device &other) = delete;
    cuda_device& operator=(cuda_device &&other) = default;

    int get_index() const noexcept;

    cuda_device_queue_pool& get_queue_pool() override;

    std::shared_ptr<device_memory_allocator>
    create_device_memory_allocator() override;

    std::shared_ptr<host_memory_allocator>
    create_host_memory_allocator() override;

    std::shared_ptr<host_to_device_transfer> 
    create_host_to_device_transfer() override;

    std::shared_ptr<device_to_host_transfer> 
    create_device_to_host_transfer() override;

    std::shared_ptr<device_copy> 
    create_device_copy() override;

    std::shared_ptr<device_event> create_device_event() override;

    std::shared_ptr<device_to_host_event> 
    create_device_to_host_event() override;

private:
    int m_device;
    cuda_device_queue_pool m_queue_pool;

}; 

} // namespace compute
} // namespace xmipp4
