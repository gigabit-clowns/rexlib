// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_plugin.hpp"

#include "hardware/cuda_device_backend.hpp"
#include "hardware/cuda_device_to_device_memory_transfer_backend.hpp"
#include "hardware/cuda_device_to_host_memory_transfer_backend.hpp"
#include "hardware/cuda_host_to_device_memory_transfer_backend.hpp"

#include <xmipp4/core/service_catalog.hpp>
#include <xmipp4/core/hardware/device_manager.hpp>
#include <xmipp4/core/hardware/memory_transfer_manager.hpp>

namespace xmipp4 
{

const std::string cuda_plugin::name = "xmipp4-cuda";

const std::string& cuda_plugin::get_name() const noexcept
{
	return name; 
}

version cuda_plugin::get_version() const noexcept
{
	return version(
		VERSION_MAJOR,
		VERSION_MINOR,
		VERSION_PATCH
	);
}

void cuda_plugin::register_at(service_catalog& catalog) const
{
	auto &device_manager = 
		catalog.get_service_manager<hardware::device_manager>();
	hardware::cuda_device_backend::register_at(device_manager);

	auto &transfer_manager = 
		catalog.get_service_manager<hardware::memory_transfer_manager>();
	hardware::cuda_device_to_device_memory_transfer_backend::register_at(
		transfer_manager
	);
	hardware::cuda_device_to_host_memory_transfer_backend::register_at(
		transfer_manager
	);
	hardware::cuda_host_to_device_memory_transfer_backend::register_at(
		transfer_manager
	);
}

} // namespace xmipp4
