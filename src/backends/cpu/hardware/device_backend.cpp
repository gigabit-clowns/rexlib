// SPDX-License-Identifier: GPL-3.0-only

#include "device_backend.hpp"

#include <rex/backends/cpu/device.hpp>

#include <rex/core/hardware/device_manager.hpp>
#include <rex/core/core_version.hpp>
#include <rex/core/system/host.hpp>

namespace rex
{
namespace cpu
{

std::string device_backend::get_name() const
{
	return "cpu";
}

version device_backend::get_version() const
{
	return get_core_version();
}

void device_backend::enumerate_devices(std::vector<std::size_t> &ids) const
{
	ids = { 0 };
}

bool device_backend::get_device_properties(
	std::size_t id, 
	device_properties &desc
) const
{
	bool result = false;

	if (id == 0)
	{
		desc.set_name(get_hostname());
		desc.set_type(device_type::cpu);
		desc.set_physical_location("");
		desc.set_total_memory_bytes(get_total_system_memory());
		desc.set_optimal_data_alignment(64); //AVX-512
		result = true;
	}

	return result;
}

std::shared_ptr<xmipp4::device> 
device_backend::create_device(std::size_t id) const
{
	if (id >= 1)
	{
		throw std::invalid_argument("Requested device id is invalid");
	}

	return std::make_shared<device>();
}

bool device_backend::register_at(xmipp4::device_manager &manager)
{
	return manager.register_backend(std::make_unique<device_backend>());
}

} // namespace cpu
} // namespace rex
