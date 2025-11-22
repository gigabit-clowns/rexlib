// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_host_pinned_memory_resource.hpp"

#include <xmipp4/core/memory/align.hpp>
#include <xmipp4/cuda/hardware/cuda_error.hpp>

#include "cuda_host_pinned_memory_heap.hpp"

#include <stdexcept>

namespace xmipp4
{
namespace hardware
{

cuda_host_pinned_memory_resource cuda_host_pinned_memory_resource::m_instance;

device* cuda_host_pinned_memory_resource::get_target_device() const noexcept
{
	return nullptr;
}

memory_resource_kind cuda_host_pinned_memory_resource::get_kind() const noexcept
{
	return memory_resource_kind::host_staging;
}

std::size_t 
cuda_host_pinned_memory_resource::get_max_heap_alignment() const noexcept
{
	return 64;
}

std::shared_ptr<memory_heap> 
cuda_host_pinned_memory_resource::create_memory_heap(
	std::size_t size, 
	std::size_t alignment
)
{   
	if (alignment > get_max_heap_alignment())
	{
		throw std::invalid_argument(
			"The alignment exceeds the maximum alignment guaranteed by the memory "
			"resource"
		);
	}

	return std::make_shared<cuda_host_pinned_memory_heap>(size);
}

cuda_host_pinned_memory_resource& 
cuda_host_pinned_memory_resource::get() noexcept
{
	return m_instance;
}

} // namespace hardware
} // namespace xmipp4
