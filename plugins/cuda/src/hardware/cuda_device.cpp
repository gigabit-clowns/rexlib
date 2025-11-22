// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/cuda/hardware/cuda_device.hpp>

#include <xmipp4/cuda/hardware/cuda_device_queue.hpp>
#include <xmipp4/cuda/hardware/cuda_event.hpp>

#include <xmipp4/core/platform/assert.hpp>

#include "cuda_host_pinned_memory_resource.hpp"
#include "cuda_device_memory_resource.hpp"

#include <memory>

namespace xmipp4
{
namespace hardware
{

cuda_device::cuda_device(int device_index)
	: m_device_index(device_index)
	, m_device_local_memory_resource(
		std::make_unique<cuda_device_memory_resource>(*this)
	)
{
}

int cuda_device::get_index() const noexcept
{
	return m_device_index;
}

void cuda_device::enumerate_memory_resources(
	std::vector<memory_resource*> &resources
)
{
	XMIPP4_ASSERT( m_device_local_memory_resource );
	resources = {
		m_device_local_memory_resource.get(),
		&cuda_host_pinned_memory_resource::get()
	};
}

std::shared_ptr<device_queue>
cuda_device::create_device_queue()
{
	return std::make_shared<cuda_device_queue>(*this);
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

} // namespace hardware
} // namespace xmipp4
