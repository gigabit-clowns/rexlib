// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>
#include <backends/cpu/hardware/cpu_command_queue.hpp>
#include <backends/cpu/loops/multidimensional_index.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>

#include <rexlib/backends/cpu/cpu_program.hpp>
#include <rexlib/backends/cpu/thread_pool.hpp>
#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/operation/operand_names.hpp>
#include <rexlib/core/operation/operation.hpp>
#include <rexlib/core/operation/operation_arity.hpp>
#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/core/operation/operation_descriptor.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include "../serial_pool.hpp"

#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/program.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/numerical_cast.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>

#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

#include "../../../core/operation/mock/mock_operation.hpp"

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
	ops::binary_elementwise_shape_rule,
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

operand_descriptor make_float32_signature(std::size_t count)
{
	const std::array<std::size_t, 1> extents { count };
	return operand_descriptor(
		strided_layout::make_contiguous_layout(make_span(extents)),
		numerical_type::float32,
		nullptr
	);
}

std::shared_ptr<host_buffer> make_float32_buffer(std::size_t count)
{
	return std::make_shared<host_buffer>(count * sizeof(float), alignof(float));
}

} // anonymous namespace

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
	cpu_command_queue queue(get_serial_pool());

	const std::size_t count = 4;
	const std::vector<operand_descriptor> outputs {
		make_float32_signature(count)
	};
	const std::vector<operand_descriptor> inputs {
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

	auto &executable = dynamic_cast<cpu_program&>(*program);
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
	cpu_command_queue queue(get_serial_pool());

	// The rejection names the operation it was handed, so the mock has to
	// be able to answer that much.
	static REXLIB_CONST_CONSTEXPR auto output_names =
		make_operand_names("result");
	static REXLIB_CONST_CONSTEXPR auto input_names =
		make_operand_names("left", "right");
	static const operation_descriptor descriptor = make_operation_descriptor(
		"rexlib.test",
		"mock",
		output_names,
		input_names
	);
	ALLOW_CALL(operation, get_descriptor()).LR_RETURN(descriptor);

	const std::vector<operand_descriptor> outputs {
		make_float32_signature(4)
	};
	const std::vector<operand_descriptor> inputs {
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
	"elementwise_program_builder::build throws on a wrong output descriptor "
	"count",
	"[elementwise_program_builder]"
)
{
	const test_builder builder;
	const test_binary_operation operation;
	cpu_command_queue queue(get_serial_pool());

	// The operation expects exactly one output.
	const std::vector<operand_descriptor> outputs {
		make_float32_signature(4),
		make_float32_signature(4)
	};
	const std::vector<operand_descriptor> inputs {
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
	"elementwise_program_builder::build throws on a wrong input descriptor "
	"count",
	"[elementwise_program_builder]"
)
{
	const test_builder builder;
	const test_binary_operation operation;
	cpu_command_queue queue(get_serial_pool());

	const std::vector<operand_descriptor> outputs {
		make_float32_signature(4)
	};
	// The operation expects exactly two inputs.
	const std::vector<operand_descriptor> inputs {
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

namespace
{

// An operation writing its output from nothing but where each element sits.
REXLIB_DECLARE_OPERATION(
	test_coordinates,
	ops::ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS(),
	ops::nullary_elementwise_shape_rule,
	ops::nullary_free_rule<real_arithmetic_type_domain>
);

// Writes the coordinates of an element as the digits of a base ten number.
struct coordinate_kernel
{
	template <typename T>
	void operator()(
		T *result,
		const multidimensional_index &index
	) const noexcept
	{
		*result = numerical_cast<T>(10*index[0] + index[1]);
	}
};

using coordinate_builder = multidimensional_indexed_elementwise_program_builder<
	test_coordinates_operation,
	default_kernel_factory<coordinate_kernel>
>;

} // anonymous namespace

TEST_CASE(
	"elementwise_program_builder hands the kernel the coordinates of every "
	"element of an output stored column by column",
	"[elementwise_program_builder]"
)
{
	const coordinate_builder builder;
	const test_coordinates_operation operation;
	cpu_command_queue queue(get_serial_pool());

	// Element (i, j) of a 2x3 output stored column by column sits at i + 2j.
	const std::array<std::size_t, 2> extents { 2, 3 };
	const std::array<std::ptrdiff_t, 2> strides { 1, 2 };
	const std::vector<operand_descriptor> outputs {
		operand_descriptor(
			strided_layout::make_custom_layout(
				make_span(extents),
				make_span(strides)
			),
			numerical_type::float32,
			nullptr
		)
	};
	const std::vector<operand_descriptor> inputs;

	const auto program = builder.build(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue,
		nullptr
	);
	REQUIRE( program != nullptr );

	const auto result_buffer = make_float32_buffer(6);
	const std::vector<std::shared_ptr<buffer>> output_operands {
		result_buffer
	};
	const std::vector<std::shared_ptr<const buffer>> input_operands;
	const std::vector<std::shared_ptr<buffer>> scratch_operands;

	auto &executable = dynamic_cast<cpu_program&>(*program);
	executable.execute(
		make_span(output_operands),
		make_span(input_operands),
		make_span(scratch_operands),
		*get_serial_pool()
	);

	const auto *result_ptr =
		static_cast<const float*>(result_buffer->get_host_ptr());
	for (std::size_t i = 0; i < 2; ++i)
	{
		for (std::size_t j = 0; j < 3; ++j)
		{
			CHECK( result_ptr[i + 2*j] == static_cast<float>(10*i + j) );
		}
	}
}
