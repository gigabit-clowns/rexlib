// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <xmipp4/core/compute/device_queue_pool.hpp>

#include "cuda_device_queue.hpp"

#include <vector>

namespace xmipp4 
{
namespace compute
{

/**
 * @brief Implementation of the device_queue_pool interface to be 
 * able to obtain cuda_device_queue-s.
 * 
 */
class cuda_device_queue_pool final
    : public device_queue_pool
{
public:
    cuda_device_queue_pool(int device_index, std::size_t count);
    cuda_device_queue_pool(const cuda_device_queue_pool &other) = delete;
    cuda_device_queue_pool(cuda_device_queue_pool &&other) = default;
    ~cuda_device_queue_pool() override = default;

    cuda_device_queue_pool&
    operator=(const cuda_device_queue_pool &other) = delete;
    cuda_device_queue_pool&
    operator=(cuda_device_queue_pool &&other) = default;

    std::size_t get_size() const noexcept override;
    cuda_device_queue& get_queue(std::size_t index) override;

private:
    std::vector<cuda_device_queue> m_queues;

}; 

} // namespace compute
} // namespace xmipp4
