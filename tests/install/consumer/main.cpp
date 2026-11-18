// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/dispatch/dispatcher.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>
#include <rexlib/core/dispatch/program_service.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/device_context.hpp>
#include <rexlib/core/hardware/device_index.hpp>
#include <rexlib/core/hardware/device_service.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/library_version.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/plugin.hpp>
#include <rexlib/core/plugin_manager.hpp>
#include <rexlib/core/service_catalog.hpp>
#include <rexlib/core/version.hpp>
#include <rexlib/core/version_macros.h>
#include <rexlib/functional/creation.hpp>

#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

// Checks that the installed headers state the version of the installed
// library. Then counts to five on the CPU, which takes the library, its
// services and its CPU backend, and on the device of the sample plugin,
// which takes that plugin, installed beside the library, and the program it
// registers.

namespace
{

bool check_version()
{
	const rexlib::version library = rexlib::get_library_version();
	const rexlib::version headers(
		REXLIB_VERSION_MAJOR,
		REXLIB_VERSION_MINOR,
		REXLIB_VERSION_PATCH
	);
	std::cout << "rexlib " << library << "\n";
	if (library != headers)
	{
		std::cerr << "The headers state version " << headers << ".\n";
		return false;
	}
	return true;
}

bool load_sample_plugin(rexlib::plugin_manager &plugins)
{
	const auto directory = rexlib::get_default_plugin_directory();
	rexlib::discover_plugins(directory, plugins);
	for (std::size_t i = 0; i < plugins.get_plugin_count(); ++i)
	{
		const auto &plugin = plugins.get_plugin(i);
		if (plugin.get_name() == "rexlib-sample")
		{
			std::cout << plugin.get_name() << " " << plugin.get_version()
				<< "\n";
			return true;
		}
	}
	std::cerr << "The sample plugin is not in " << directory << ".\n";
	return false;
}

bool count_to_five(
	rexlib::service_catalog &catalog,
	const std::string &backend
)
{
	const auto devices = catalog.get_service<rexlib::device_service>();
	const auto programs = catalog.get_service<rexlib::program_service>();
	const rexlib::execution_context context(
		rexlib::device_context(
			devices->create_device_session(rexlib::device_index(backend, 0))
		),
		rexlib::make_eager_dispatcher(programs)
	);

	auto result = rexlib::arange(
		5,
		rexlib::numerical_type::int32,
		rexlib::memory_resource_affinity::host,
		context
	);

	const auto *values = static_cast<const std::int32_t*>(
		result.get_storage()->get_host_ptr()
	);
	for (std::int32_t i = 0; i < 5; ++i)
	{
		if (values[i] != i)
		{
			std::cerr << "arange did not count to five on " << backend
				<< ".\n";
			return false;
		}
	}
	return true;
}

int run()
{
	if (!check_version())
	{
		return 1;
	}

	rexlib::plugin_manager plugins;
	if (!load_sample_plugin(plugins))
	{
		return 1;
	}

	rexlib::service_catalog catalog;
	rexlib::register_all_plugins_at(plugins, catalog);

	if (!count_to_five(catalog, "cpu") || !count_to_five(catalog, "sample"))
	{
		return 1;
	}
	return 0;
}

} // anonymous namespace

int main()
{
	try
	{
		return run();
	}
	catch (const std::exception &error)
	{
		std::cerr << error.what() << "\n";
		return 1;
	}
}
