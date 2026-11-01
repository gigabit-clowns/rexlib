// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/dispatch/dispatcher.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>
#include <rexlib/core/dispatch/program_manager.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/device_context.hpp>
#include <rexlib/core/hardware/device_index.hpp>
#include <rexlib/core/hardware/device_manager.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/library_version.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/service_catalog.hpp>
#include <rexlib/functional/creation.hpp>

#include <cstdint>
#include <iostream>

// Counts to five on the CPU, which takes the library, its services and its
// CPU backend.
int main()
{
	std::cout << "rexlib " << rexlib::get_library_version() << "\n";

	rexlib::service_catalog catalog;
	const auto devices =
		catalog.get_service_manager<rexlib::device_manager>();
	const auto programs =
		catalog.get_service_manager<rexlib::program_manager>();
	const rexlib::execution_context context(
		rexlib::device_context(
			devices->create_device_session(rexlib::device_index("cpu", 0))
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
			std::cerr << "arange did not count to five.\n";
			return 1;
		}
	}

	return 0;
}
