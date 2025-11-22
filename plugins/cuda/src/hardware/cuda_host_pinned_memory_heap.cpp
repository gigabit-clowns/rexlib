// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_host_pinned_memory_heap.hpp"

#include <xmipp4/cuda/hardware/cuda_error.hpp>
#include <xmipp4/cuda/hardware/cuda_device.hpp>
#include <xmipp4/core/platform/assert.hpp>
#include <xmipp4/core/platform/constexpr.hpp>
#include <xmipp4/core/memory/align.hpp>
#include <xmipp4/core/hardware/buffer.hpp>

#include "cuda_host_pinned_memory_resource.hpp"

#include <cuda_runtime.h>

namespace xmipp4
{
namespace hardware
{

cuda_host_pinned_memory_heap::cuda_host_pinned_memory_heap(std::size_t size)
	: m_data(nullptr)
	, m_size(size)
{
	cudaMallocHost(&m_data, m_size);
	if (!m_data && size > 0)
	{
		throw std::bad_alloc();
	}
}

cuda_host_pinned_memory_heap::~cuda_host_pinned_memory_heap()
{
	cudaFreeHost(m_data);
}

std::size_t cuda_host_pinned_memory_heap::get_size() const noexcept
{
	return m_size;
}

std::shared_ptr<buffer> cuda_host_pinned_memory_heap::create_buffer(
	std::size_t offset, 
	std::size_t size,
	std::unique_ptr<buffer_sentinel> sentinel
)
{
	if (offset + size > m_size)
	{
		throw std::out_of_range("Allocation exceeds heap bounds");
	}

	return std::make_shared<buffer>(
		memory::offset_bytes(m_data, offset),
		size,
		cuda_host_pinned_memory_resource::get(),
		std::move(sentinel)
	);
}

} // namespace hardware
} // namespace xmipp4
