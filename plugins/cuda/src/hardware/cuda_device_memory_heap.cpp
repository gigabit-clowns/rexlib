// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_device_memory_heap.hpp"

#include <xmipp4/cuda/hardware/cuda_error.hpp>
#include <xmipp4/cuda/hardware/cuda_device.hpp>
#include <xmipp4/cuda/hardware/cuda_buffer.hpp>
#include <xmipp4/core/platform/assert.hpp>
#include <xmipp4/core/platform/constexpr.hpp>
#include <xmipp4/core/memory/align.hpp>

#include "cuda_device_memory_resource.hpp"

#include <cuda_runtime.h>

namespace xmipp4
{
namespace hardware
{

cuda_device_memory_heap::cuda_device_memory_heap(
	cuda_device_memory_resource &resource,
	std::size_t size
)
	: m_resource(resource)
	, m_data(nullptr)
	, m_size(size)
{
	const auto *device = resource.get_target_device();
	XMIPP4_ASSERT( device );
	const auto device_index = device->get_index();

	XMIPP4_CUDA_CHECK( cudaSetDevice(device_index) );
	cudaMalloc(&m_data, m_size);
	if (!m_data && size > 0)
	{
		throw std::bad_alloc();
	}
}

cuda_device_memory_heap::~cuda_device_memory_heap()
{
	cudaFree(m_data);
}

std::size_t cuda_device_memory_heap::get_size() const noexcept
{
	return m_size;
}

std::shared_ptr<buffer> cuda_device_memory_heap::create_buffer(
	std::size_t offset, 
	std::size_t size,
	std::unique_ptr<buffer_sentinel> sentinel
)
{
	if (offset + size > m_size)
	{
		throw std::out_of_range("Allocation exceeds heap bounds");
	}

	return std::make_shared<cuda_buffer>(
		memory::offset_bytes(m_data, offset),
		nullptr,
		size,
		m_resource,
		std::move(sentinel)
	);
}

} // namespace hardware
} // namespace xmipp4
