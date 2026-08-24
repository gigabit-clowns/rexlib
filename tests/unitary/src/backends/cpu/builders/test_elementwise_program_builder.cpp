// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>
#include <backends/cpu/hardware/command_queue.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>

#include <rexlib/core/dispatch/basic_operation.hpp>
#include <rexlib/core/dispatch/operation.hpp>
#include <rexlib/core/dispatch/operation_arity.hpp>
#include <rexlib/core/dispatch/operand_signature.hpp>
#include <rexlib/backends/cpu/program.hpp>
#include <rexlib/backends/cpu/thread_pool.hpp>

#include "../serial_pool.hpp"

#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/program.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>

#include <rexlib/ops/policies/elementwise_operation_shape_policy.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

#include "../../../core/dispatch/mock/mock_operation.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{


// A self-contained binary elementwise operation.
REXLIB_DECLARE_OPERATION(
	test_binary,
	ops::ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("left", "right"),
	ops::elementwise_operation_shape_policy,
	ops::binary_homogeneous_rule<>
);

// Per-element kernel adding the two inputs into the output.
struct sum_kernel
{
	template <typename T>
	void operator()(T *result, const T *x, const T *y) const noexcept
	{
		store(result, load(x) + load(y));
	}
};

using test_builder = elementwise_program_builder<
	test_binary_operation,
	default_kernel_factory<sum_kernel>
>;

operand_signature make_float32_signature(std::size_t count)
{
	const std::array<std::size_t, 1> extents { count };
	return operand_signature(
		strided_layout::make_contiguous_layout(make_span(extents)),
		numerical_type::float32,
		nullptr
	);
}

std::shared_ptr<host_buffer> make_float32_buffer(std::size_t count)
{
	return std::make_shared<host_buffer>(count * sizeof(float), alignof(float));
}

} // namespace

TEST_CASE(
	"elementwise_program_builder reports its target operation id",
	"[elementwise_program_builder]"
)
{
	const test_builder builder;

	CHECK(
		builder.get_operation_id() ==
		operation_id::of<test_binary_operation>()
	);
}

TEST_CASE(
	"elementwise_program_builder builds a program applying the kernel "
	"elementwise",
	"[elementwise_program_builder]"
)
{
	const test_builder builder;
	const test_binary_operation operation;
	cpu::command_queue queue(get_serial_pool());

	const std::size_t count = 4;
	const std::vector<operand_signature> outputs {
		make_float32_signature(count)
	};
	const std::vector<operand_signature> inputs {
		make_float32_signature(count),
		make_float32_signature(count)
	};

	const auto program = builder.build(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue,
		nullptr
	);
	REQUIRE( program != nullptr );

	const auto result_buffer = make_float32_buffer(count);
	const auto left_buffer = make_float32_buffer(count);
	const auto right_buffer = make_float32_buffer(count);

	auto *result_ptr = static_cast<float*>(result_buffer->get_host_ptr());
	auto *left_ptr = static_cast<float*>(left_buffer->get_host_ptr());
	auto *right_ptr = static_cast<float*>(right_buffer->get_host_ptr());
	for (std::size_t i = 0; i < count; ++i)
	{
		left_ptr[i] = static_cast<float>(i);
		right_ptr[i] = static_cast<float>(2 * i);
	}

	const std::vector<std::shared_ptr<buffer>> output_operands {
		result_buffer
	};
	const std::vector<std::shared_ptr<const buffer>> input_operands {
		left_buffer,
		right_buffer
	};
	const std::vector<std::shared_ptr<buffer>> scratch_operands;

	auto &executable = dynamic_cast<cpu::program&>(*program);
	executable.execute(
		make_span(output_operands),
		make_span(input_operands),
		make_span(scratch_operands),
		*get_serial_pool()
	);

	for (std::size_t i = 0; i < count; ++i)
	{
		CHECK( result_ptr[i] == static_cast<float>(3 * i) );
	}
}

TEST_CASE(
	"elementwise_program_builder::build throws on an unexpected operation type",
	"[elementwise_program_builder]"
)
{
	const test_builder builder;
	mock_operation operation;
	cpu::command_queue queue(get_serial_pool());

	// The rejection names the operation it was handed, so the mock has to
	// be able to answer that much.
	ALLOW_CALL(operation, get_name()).RETURN("mock");

	const std::vector<operand_signature> outputs {
		make_float32_signature(4)
	};
	const std::vector<operand_signature> inputs {
		make_float32_signature(4),
		make_float32_signature(4)
	};

	CHECK_THROWS_AS(
		builder.build(
			operation,
			make_span(outputs),
			make_span(inputs),
			queue,
			nullptr
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"elementwise_program_builder::build throws on a wrong output signature "
	"count",
	"[elementwise_program_builder]"
)
{
	const test_builder builder;
	const test_binary_operation operation;
	cpu::command_queue queue(get_serial_pool());

	// The operation expects exactly one output.
	const std::vector<operand_signature> outputs {
		make_float32_signature(4),
		make_float32_signature(4)
	};
	const std::vector<operand_signature> inputs {
		make_float32_signature(4),
		make_float32_signature(4)
	};

	CHECK_THROWS_AS(
		builder.build(
			operation,
			make_span(outputs),
			make_span(inputs),
			queue,
			nullptr
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"elementwise_program_builder::build throws on a wrong input signature "
	"count",
	"[elementwise_program_builder]"
)
{
	const test_builder builder;
	const test_binary_operation operation;
	cpu::command_queue queue(get_serial_pool());

	const std::vector<operand_signature> outputs {
		make_float32_signature(4)
	};
	// The operation expects exactly two inputs.
	const std::vector<operand_signature> inputs {
		make_float32_signature(4)
	};

	CHECK_THROWS_AS(
		builder.build(
			operation,
			make_span(outputs),
			make_span(inputs),
			queue,
			nullptr
		),
		std::invalid_argument
	);
}
