// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/cuda/compute/cuda_device.hpp>

#include <xmipp4/cuda/compute/cuda_device_queue.hpp>
#include <xmipp4/cuda/compute/cuda_device_memory_allocator.hpp>
#include <xmipp4/cuda/compute/cuda_host_memory_allocator.hpp>
#include <xmipp4/cuda/compute/cuda_device_to_host_transfer.hpp>
#include <xmipp4/cuda/compute/cuda_host_to_device_transfer.hpp>
#include <xmipp4/cuda/compute/cuda_device_copy.hpp>
#include <xmipp4/cuda/compute/cuda_event.hpp>

#include <xmipp4/core/compute/device_create_parameters.hpp>

#include <memory>

namespace xmipp4
{
namespace compute
{

cuda_device::cuda_device(int device, const device_create_parameters &params)
    : m_device(device)
    , m_queue_pool(device, params.get_desired_queue_count())
{
}

int cuda_device::get_index() const noexcept
{
    return m_device;
}

cuda_device_queue_pool& cuda_device::get_queue_pool()
{
    return m_queue_pool;
}

std::shared_ptr<device_memory_allocator> 
cuda_device::create_device_memory_allocator()
{
    return std::make_shared<cuda_device_memory_allocator>(*this);
}

std::shared_ptr<host_memory_allocator> 
cuda_device::create_host_memory_allocator()
{
    return std::make_shared<cuda_host_memory_allocator>();
}

std::shared_ptr<host_to_device_transfer> 
cuda_device::create_host_to_device_transfer()
{
    return std::make_shared<cuda_host_to_device_transfer>();
}

std::shared_ptr<device_to_host_transfer> 
cuda_device::create_device_to_host_transfer()
{
    return std::make_shared<cuda_device_to_host_transfer>();
}

std::shared_ptr<device_copy> 
cuda_device::create_device_copy()
{
    return std::make_shared<cuda_device_copy>();
}

std::shared_ptr<device_event> cuda_device::create_device_event()
{
    return std::make_shared<cuda_event>();
}

std::shared_ptr<device_to_host_event> 
cuda_device::create_device_to_host_event()
{
    return std::make_shared<cuda_event>();
}

} // namespace compute
} // namespace xmipp4
