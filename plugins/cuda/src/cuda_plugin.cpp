// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_plugin.hpp"

#include "hardware/cuda_device_backend.hpp"

#include <rexlib/core/hardware/device_manager.hpp>
#include <rexlib/core/service_catalog.hpp>

namespace rexlib
{
namespace cuda
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
	cuda_device_backend::register_at(*device_manager);
}

} // namespace cuda
} // namespace rexlib
