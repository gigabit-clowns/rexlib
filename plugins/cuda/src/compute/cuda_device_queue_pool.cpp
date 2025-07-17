// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/cuda/compute/cuda_device_queue_pool.hpp>

#include <xmipp4/cuda/compute/cuda_error.hpp>
#include <xmipp4/cuda/compute/cuda_device.hpp>

#include <cuda_runtime.h>

namespace xmipp4
{
namespace compute
{

cuda_device_queue_pool::cuda_device_queue_pool(int device_index, std::size_t count)
{
    XMIPP4_CUDA_CHECK( cudaSetDevice(device_index) );
    m_queues.resize(count);
}

std::size_t cuda_device_queue_pool::get_size() const noexcept
{
    return m_queues.size();
}

cuda_device_queue& cuda_device_queue_pool::get_queue(std::size_t index)
{
    return m_queues.at(index);
}

} // namespace compute
} // namespace xmipp4
