// SPDX-License-Identifier: GPL-3.0-only

#include "plugin.hpp"

#include "hardware/device_backend.hpp"

#include <rexlib/core/hardware/device_manager.hpp>
#include <rexlib/core/service_catalog.hpp>

namespace rexlib
{

const std::string cuda_plugin::name = "rexlib-cuda";

const std::string& cuda_plugin::get_name() const noexcept
{
	return name;
}

version cuda_plugin::get_version() const noexcept
{
	return version(
		REXLIB_CUDA_VERSION_MAJOR,
		REXLIB_CUDA_VERSION_MINOR,
		REXLIB_CUDA_VERSION_PATCH
	);
}

void cuda_plugin::register_at(service_catalog& catalog) const
{
	const auto device_manager = catalog.get_service_manager<rexlib::device_manager>();
	cuda::device_backend::register_at(*device_manager);
}

} // namespace rexlib
