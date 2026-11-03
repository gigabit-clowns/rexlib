// SPDX-License-Identifier: GPL-3.0-only

#include "cpu_plugin.hpp"

#include <backends/cpu/builders/cpu_program_builder_registry.hpp>
#include <backends/cpu/hardware/cpu_device_backend.hpp>
#include <core/builtin_plugin_registration.hpp>

#include <rexlib/core/dispatch/program_manager.hpp>
#include <rexlib/core/hardware/device_manager.hpp>
#include <rexlib/core/library_version.hpp>
#include <rexlib/core/service_catalog.hpp>

#include <string>

namespace rexlib
{
namespace cpu
{

namespace
{

const builtin_plugin_registration<cpu_plugin> registration(
	get_builtin_plugin_registry()
);

} // anonymous namespace

const std::string& cpu_plugin::get_name() const noexcept
{
	static const std::string name("rexlib-cpu");
	return name;
}

version cpu_plugin::get_version() const noexcept
{
	return get_library_version();
}

void cpu_plugin::register_at(service_catalog &catalog) const
{
	cpu_device_backend::register_at(
		*catalog.get_service_manager<device_manager>()
	);
	get_cpu_program_builder_registry().register_all(
		*catalog.get_service_manager<program_manager>()
	);
}

} // namespace cpu
} // namespace rexlib
