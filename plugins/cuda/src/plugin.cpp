// SPDX-License-Identifier: GPL-3.0-only

#include "plugin.hpp"

#include "hardware/device_backend.hpp"

#include <xmipp4/core/hardware/device_manager.hpp>
#include <xmipp4/core/service_catalog.hpp>

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
	const auto device_manager = catalog.get_service_manager<xmipp4::device_manager>();
	cuda::device_backend::register_at(*device_manager);
}

} // namespace xmipp4
