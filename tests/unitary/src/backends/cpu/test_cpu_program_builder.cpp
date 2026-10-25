// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/backends/cpu/cpu_program_builder.hpp>
#include <backends/cpu/hardware/cpu_command_queue.hpp>

#include "mock/mock_program_builder.hpp"
#include "serial_pool.hpp"

#include <rexlib/core/backend_priority.hpp>
#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/hardware/memory_resource_kind.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>

#include "../../core/operation/mock/mock_operation.hpp"
#include "../../core/hardware/mock/mock_memory_resource.hpp"
#include "../../core/hardware/mock/mock_command_queue.hpp"

#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

operand_descriptor make_operand_descriptor(const memory_resource *resource)
{
	return operand_descriptor(
		strided_layout(),
		numerical_type::float32,
		resource
	);
}

} // anonymous namespace


TEST_CASE(
	"cpu_program_builder get_suitability returns normal when every "
	"descriptor is host accessible and the queue is a CPU queue",
	"[cpu_program_builder]"
)
{
	const cpu::mock_program_builder builder;
	const mock_operation operation;

	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);

	const std::vector<operand_descriptor> outputs {
		make_operand_descriptor(&host_resource)
	};
	const std::vector<operand_descriptor> inputs {
		make_operand_descriptor(&host_resource)
	};

	cpu_command_queue queue(get_serial_pool());

	const auto priority = builder.get_suitability(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue
	);

	CHECK( priority == backend_priority::normal );
}

TEST_CASE(
	"cpu_program_builder get_suitability returns unsupported when "
	"any output descriptor is not host accessible",
	"[cpu_program_builder]"
)
{
	const cpu::mock_program_builder builder;
	const mock_operation operation;

	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);
	mock_memory_resource device_resource;
	ALLOW_CALL(device_resource, get_kind())
		.RETURN(memory_resource_kind::device_local);

	// The second output resides on device-local memory, so the whole set is
	// rejected even though the first one is host accessible.
	const std::vector<operand_descriptor> outputs {
		make_operand_descriptor(&host_resource),
		make_operand_descriptor(&device_resource)
	};
	const std::vector<operand_descriptor> inputs {
		make_operand_descriptor(&host_resource)
	};

	cpu_command_queue queue(get_serial_pool());

	const auto priority = builder.get_suitability(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue
	);

	CHECK( priority == backend_priority::unsupported );
}

TEST_CASE(
	"cpu_program_builder get_suitability returns unsupported when "
	"any input descriptor is not host accessible",
	"[cpu_program_builder]"
)
{
	const cpu::mock_program_builder builder;
	const mock_operation operation;

	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);
	mock_memory_resource device_resource;
	ALLOW_CALL(device_resource, get_kind())
		.RETURN(memory_resource_kind::device_local);

	const std::vector<operand_descriptor> outputs {
		make_operand_descriptor(&host_resource)
	};
	const std::vector<operand_descriptor> inputs {
		make_operand_descriptor(&device_resource)
	};

	cpu_command_queue queue(get_serial_pool());

	const auto priority = builder.get_suitability(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue
	);

	CHECK( priority == backend_priority::unsupported );
}

TEST_CASE(
	"cpu_program_builder get_suitability returns unsupported when a "
	"descriptor has no memory resource",
	"[cpu_program_builder]"
)
{
	const cpu::mock_program_builder builder;
	const mock_operation operation;

	// A null memory resource cannot be host accessible.
	const std::vector<operand_descriptor> outputs {
		make_operand_descriptor(nullptr)
	};
	const std::vector<operand_descriptor> inputs;

	cpu_command_queue queue(get_serial_pool());

	const auto priority = builder.get_suitability(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue
	);

	CHECK( priority == backend_priority::unsupported );
}

TEST_CASE(
	"cpu_program_builder get_suitability returns unsupported when "
	"the queue is not a CPU queue",
	"[cpu_program_builder]"
)
{
	const cpu::mock_program_builder builder;
	const mock_operation operation;

	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);

	const std::vector<operand_descriptor> outputs {
		make_operand_descriptor(&host_resource)
	};
	const std::vector<operand_descriptor> inputs {
		make_operand_descriptor(&host_resource)
	};

	// A non-CPU queue fails the try_cast even with valid descriptors.
	mock_command_queue queue;

	const auto priority = builder.get_suitability(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue
	);

	CHECK( priority == backend_priority::unsupported );
}

TEST_CASE(
	"cpu_program_builder get_suitability returns normal for empty "
	"descriptor lists on a CPU queue",
	"[cpu_program_builder]"
)
{
	const cpu::mock_program_builder builder;
	const mock_operation operation;

	// With no descriptors to reject, only the queue kind decides suitability.
	const std::vector<operand_descriptor> outputs;
	const std::vector<operand_descriptor> inputs;

	cpu_command_queue queue(get_serial_pool());

	const auto priority = builder.get_suitability(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue
	);

	CHECK( priority == backend_priority::normal );
}
