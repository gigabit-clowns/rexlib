// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/backends/cpu/program_builder.hpp>
#include <backends/cpu/hardware/command_queue.hpp>

#include "serial_pool.hpp"

#include <rex/core/dispatch/operand_signature.hpp>
#include <rex/core/layout/strided_layout.hpp>
#include <rex/core/dispatch/operation_id.hpp>
#include <rex/core/hardware/memory_resource_kind.hpp>
#include <rex/core/backend_priority.hpp>
#include <rex/core/numerical/numerical_type.hpp>
#include <rex/core/span.hpp>

#include "../../core/dispatch/mock/mock_operation.hpp"
#include "../../core/hardware/mock/mock_memory_resource.hpp"
#include "../../core/hardware/mock/mock_command_queue.hpp"

#include <memory>
#include <vector>

using namespace rex;
using namespace rex::cpu;

namespace
{

/**
 * @brief Concrete builder used to exercise get_suitability.
 *
 * cpu::program_builder only overrides get_suitability and stays
 * abstract, so this subclass provides trivial stubs for the remaining pure
 * virtuals. Neither is invoked by the tests.
 */
class test_cpu_program_builder final
	: public cpu::program_builder
{
public:
	operation_id get_operation_id() const noexcept override
	{
		return operation_id::of<test_cpu_program_builder>();
	}

	std::shared_ptr<xmipp4::program> build(
		const operation & /*operation*/,
		span<const operand_signature> /*output_signatures*/,
		span<const operand_signature> /*input_signatures*/,
		xmipp4::command_queue & /*queue*/,
		program_cache * /*cache*/
	) const override
	{
		return nullptr;
	}
};

operand_signature make_signature(const memory_resource *resource)
{
	return operand_signature(
		strided_layout(),
		numerical_type::float32,
		resource
	);
}

} // namespace


TEST_CASE(
	"cpu::program_builder get_suitability returns normal when every "
	"signature is host accessible and the queue is a CPU queue",
	"[cpu::program_builder]"
)
{
	const test_cpu_program_builder builder;
	const mock_operation operation;

	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);

	const std::vector<operand_signature> outputs {
		make_signature(&host_resource)
	};
	const std::vector<operand_signature> inputs {
		make_signature(&host_resource)
	};

	cpu::command_queue queue(get_serial_pool());

	const auto priority = builder.get_suitability(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue
	);

	CHECK( priority == backend_priority::normal );
}

TEST_CASE(
	"cpu::program_builder get_suitability returns unsupported when "
	"any output signature is not host accessible",
	"[cpu::program_builder]"
)
{
	const test_cpu_program_builder builder;
	const mock_operation operation;

	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);
	mock_memory_resource device_resource;
	ALLOW_CALL(device_resource, get_kind())
		.RETURN(memory_resource_kind::device_local);

	// The second output resides on device-local memory, so the whole set is
	// rejected even though the first one is host accessible.
	const std::vector<operand_signature> outputs {
		make_signature(&host_resource),
		make_signature(&device_resource)
	};
	const std::vector<operand_signature> inputs {
		make_signature(&host_resource)
	};

	cpu::command_queue queue(get_serial_pool());

	const auto priority = builder.get_suitability(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue
	);

	CHECK( priority == backend_priority::unsupported );
}

TEST_CASE(
	"cpu::program_builder get_suitability returns unsupported when "
	"any input signature is not host accessible",
	"[cpu::program_builder]"
)
{
	const test_cpu_program_builder builder;
	const mock_operation operation;

	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);
	mock_memory_resource device_resource;
	ALLOW_CALL(device_resource, get_kind())
		.RETURN(memory_resource_kind::device_local);

	const std::vector<operand_signature> outputs {
		make_signature(&host_resource)
	};
	const std::vector<operand_signature> inputs {
		make_signature(&device_resource)
	};

	cpu::command_queue queue(get_serial_pool());

	const auto priority = builder.get_suitability(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue
	);

	CHECK( priority == backend_priority::unsupported );
}

TEST_CASE(
	"cpu::program_builder get_suitability returns unsupported when a "
	"signature has no memory resource",
	"[cpu::program_builder]"
)
{
	const test_cpu_program_builder builder;
	const mock_operation operation;

	// A null memory resource cannot be host accessible.
	const std::vector<operand_signature> outputs {
		make_signature(nullptr)
	};
	const std::vector<operand_signature> inputs;

	cpu::command_queue queue(get_serial_pool());

	const auto priority = builder.get_suitability(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue
	);

	CHECK( priority == backend_priority::unsupported );
}

TEST_CASE(
	"cpu::program_builder get_suitability returns unsupported when "
	"the queue is not a CPU queue",
	"[cpu::program_builder]"
)
{
	const test_cpu_program_builder builder;
	const mock_operation operation;

	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);

	const std::vector<operand_signature> outputs {
		make_signature(&host_resource)
	};
	const std::vector<operand_signature> inputs {
		make_signature(&host_resource)
	};

	// A non-CPU queue fails the try_cast even with valid signatures.
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
	"cpu::program_builder get_suitability returns normal for empty "
	"signature lists on a CPU queue",
	"[cpu::program_builder]"
)
{
	const test_cpu_program_builder builder;
	const mock_operation operation;

	// With no signatures to reject, only the queue kind decides suitability.
	const std::vector<operand_signature> outputs;
	const std::vector<operand_signature> inputs;

	cpu::command_queue queue(get_serial_pool());

	const auto priority = builder.get_suitability(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue
	);

	CHECK( priority == backend_priority::normal );
}
