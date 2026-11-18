// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/cpu_plugin.hpp>

#include <backends/cpu/hardware/cpu_command_queue.hpp>

#include "serial_pool.hpp"

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/dispatch/program_service.hpp>
#include <rexlib/core/hardware/device_service.hpp>
#include <rexlib/core/hardware/memory_resource_kind.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/library_version.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/scalar_value.hpp>
#include <rexlib/core/service_catalog.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/ops/assignment/fill_operation.hpp>

#include "../../core/hardware/mock/mock_memory_resource.hpp"

#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

TEST_CASE(
	"cpu_plugin is named rexlib-cpu",
	"[cpu_plugin]"
)
{
	CHECK( cpu_plugin().get_name() == "rexlib-cpu" );
}

TEST_CASE(
	"cpu_plugin has the version of the library",
	"[cpu_plugin]"
)
{
	CHECK( cpu_plugin().get_version() == get_library_version() );
}

TEST_CASE(
	"cpu_plugin registers the CPU device backend at a catalog",
	"[cpu_plugin]"
)
{
	service_catalog catalog(false);
	std::vector<std::string> backends;

	cpu_plugin().register_at(catalog);
	catalog.get_service<device_service>()->enumerate_backends(backends);

	CHECK( backends == std::vector<std::string>{ "cpu" } );
}

TEST_CASE(
	"cpu_plugin registers the CPU's program builders at a catalog",
	"[cpu_plugin]"
)
{
	service_catalog catalog(false);
	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);
	const std::vector<operand_descriptor> outputs {
		operand_descriptor(
			strided_layout(),
			numerical_type::float32,
			&host_resource
		)
	};
	cpu_command_queue queue(get_serial_pool());

	cpu_plugin().register_at(catalog);
	const auto program = catalog.get_service<program_service>()->build(
		ops::fill_operation(scalar_value(1.0F)),
		make_span(outputs),
		span<const operand_descriptor>(),
		queue
	);

	CHECK( program != nullptr );
}
