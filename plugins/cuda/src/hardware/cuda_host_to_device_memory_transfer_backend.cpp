// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_host_to_device_memory_transfer_backend.hpp"

#include <xmipp4/core/hardware/memory_resource.hpp>
#include <xmipp4/core/hardware/memory_transfer_manager.hpp>

#include "cuda_device_memory_resource.hpp"
#include "cuda_host_pinned_memory_resource.hpp"
#include "cuda_host_to_device_memory_transfer.hpp"

#include <stdexcept>

namespace xmipp4
{
namespace hardware
{

backend_priority cuda_host_to_device_memory_transfer_backend::get_suitability(
	const memory_resource& source,
	const memory_resource& destination
) const noexcept
{
	if (!dynamic_cast<const cuda_device_memory_resource*>(&destination))
	{
		return backend_priority::unsupported;
	}

	if (&source == &cuda_host_pinned_memory_resource::get())
	{
		return backend_priority::optimal;
	}

	if (is_host_accessible(source.get_kind()))
	{
		return backend_priority::normal;
	}

	return backend_priority::unsupported;
}

std::shared_ptr<memory_transfer> 
cuda_host_to_device_memory_transfer_backend::create_transfer(
	const memory_resource& source,
	const memory_resource& destination
) const
{
	if (get_suitability(source, destination) == backend_priority::unsupported)
	{
		throw std::invalid_argument(
			"Unsupported transfer requested for this backend"
		);
	}

	return std::make_shared<cuda_host_to_device_memory_transfer>();
}

bool cuda_host_to_device_memory_transfer_backend::register_at(
	memory_transfer_manager &manager
)
{
	return manager.register_backend(
		std::make_unique<cuda_host_to_device_memory_transfer_backend>()
	);
}

} // namespace hardware
} // namespace xmipp4
