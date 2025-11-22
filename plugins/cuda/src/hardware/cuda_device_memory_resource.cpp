// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_device_memory_resource.hpp"

#include <xmipp4/cuda/hardware/cuda_error.hpp>
#include <xmipp4/core/memory/align.hpp>

#include "cuda_device_memory_heap.hpp"

namespace xmipp4
{
namespace hardware
{

cuda_device_memory_resource::cuda_device_memory_resource(
	cuda_device &device
) noexcept
	: m_device(device)
{
}

cuda_device* cuda_device_memory_resource::get_target_device() const noexcept
{
	return &(m_device.get());
}

memory_resource_kind cuda_device_memory_resource::get_kind() const noexcept
{
	return memory_resource_kind::device_local;
}

std::size_t cuda_device_memory_resource::get_max_heap_alignment() const noexcept
{
	return 256;
}

std::shared_ptr<memory_heap> cuda_device_memory_resource::create_memory_heap(
	std::size_t size,
	std::size_t alignment
)
{
	if (alignment > get_max_heap_alignment())
	{
		throw std::invalid_argument(
			"alignment exceeds the maximum alignment guaranteed by the memory "
			"resource"
		);
	}

	return std::make_shared<cuda_device_memory_heap>(*this, size);
}

} // namespace hardware
} // namespace xmipp4
