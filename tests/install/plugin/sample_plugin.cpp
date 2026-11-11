// SPDX-License-Identifier: GPL-3.0-only

#include "sample_plugin.hpp"

#include "sample_device_backend.hpp"
#include "sample_program_builder_registry.hpp"

#include <rexlib/core/dispatch/program_service.hpp>
#include <rexlib/core/hardware/device_service.hpp>
#include <rexlib/core/service_catalog.hpp>

#include <memory>

namespace sample
{

const std::string& sample_plugin::get_name() const noexcept
{
	static const std::string name("rexlib-sample");
	return name;
}

rexlib::version sample_plugin::get_version() const noexcept
{
	return rexlib::version(
		SAMPLE_VERSION_MAJOR,
		SAMPLE_VERSION_MINOR,
		SAMPLE_VERSION_PATCH
	);
}

void sample_plugin::register_at(rexlib::service_catalog &catalog) const
{
	catalog.get_service<rexlib::device_service>()->register_backend(
		std::make_unique<sample_device_backend>()
	);
	get_sample_program_builder_registry().register_all(
		*catalog.get_service<rexlib::program_service>()
	);
}

} // namespace sample
