// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <backends/cpu/skeletons/elementwise/elementwise_program_builder.hpp>
#include <backends/cpu/skeletons/default_kernel_factory.hpp>
#include <backends/cpu/numerical/load_store.hpp>
#include <backends/cpu/hardware/cpu_command_queue.hpp>
#include <backends/cpu/loops/indices/multidimensional_index.hpp>

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

#include "../../hardware/mock/serial_pool.hpp"

#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/program.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/numerical_cast.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/core/operation/shape_rule/core_dimensions.hpp>
#include <rexlib/core/operation/shape_rule/extent_pivot.hpp>
#include <rexlib/core/operation/shape_rule/extent_slot.hpp>
#include <rexlib/core/operation/shape_rule/operand_shape_rule.hpp>
#include <rexlib/core/operation/shape_rule/trailing_core.hpp>

#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

#include "../../../../core/operation/mock/mock_operation.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
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

// An elementwise operation over images, from a pair of coefficients per
// image: a core of its own the outputs do not carry.
using test_affine_shape_rule = operand_shape_rule<
	type_list<extent_from_inputs<>, extent_from_inputs<>, extent_fixed<2>>,
	type_list<core_dimensions<axis_same_as<0>, axis_same_as<1>>>,
	type_list<
		core_dimensions<axis_same_as<0>, axis_same_as<1>>,
		core_dimensions<axis_same_as<2>>
	>,
	trailing_core
>;

REXLIB_DECLARE_OPERATION(
	test_affine,
	ops::ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value", "coefficients"),
	test_affine_shape_rule,
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

/**
 * @brief Writes a scale times the value plus an offset, plus the index it is
 * handed: its linear value, or 10 times the row plus the column.
 */
class affine_writer
{
public:
	affine_writer(float scale, float offset) noexcept
		: m_scale(scale)
		, m_offset(offset)
	{
	}

	void operator()(
		float *result,
		const float *value,
		std::size_t index
	) const noexcept
	{
		*result = m_scale * *value + m_offset + static_cast<float>(index);
	}

	void operator()(
		float *result,
		const float *value,
		const multidimensional_index &index
	) const noexcept
	{
		*result = m_scale * *value + m_offset +
			static_cast<float>(10 * index[0] + index[1]);
	}

private:
	float m_scale;
	float m_offset;
};

/**
 * @brief Makes the kernel of an image from its coefficients.
 */
class affine_maker
{
public:
	explicit affine_maker(std::ptrdiff_t stride) noexcept
		: m_stride(stride)
	{
	}

	template <typename Inputs>
	affine_writer make_map(const Inputs &inputs) const
	{
		const auto *coefficients = std::get<1>(inputs);
		return affine_writer(coefficients[0], coefficients[m_stride]);
	}

	template <typename Inputs>
	affine_writer make_map(const Inputs &inputs, std::size_t) const
	{
		return make_map(inputs);
	}

private:
	std::ptrdiff_t m_stride;
};

template <typename... Types>
struct float32_only : std::is_same<std::tuple<Types...>, std::tuple<float>>
{
};

struct affine_maker_factory
{
	template <typename... Outs, typename... Ins>
	affine_maker operator()(
		const test_affine_operation&,
		type_list<Outs...>,
		type_list<Ins...>,
		const batched_map_plan &plan
	) const
	{
		return affine_maker(plan.get_input_core(1).get_stride(0));
	}
};

template <typename Indexing>
using affine_builder = elementwise_program_builder<
	test_affine_operation,
	affine_maker_factory,
	rule_type_dispatch<test_affine_operation::type_rule_type, float32_only>,
	Indexing
>;

/**
 * @brief Build and run a test_affine program over images and their
 * coefficients.
 */
template <typename Builder>
std::vector<float> run_affine(
	const std::vector<std::size_t> &image_extents,
	const std::vector<float> &values,
	const std::vector<std::size_t> &coefficient_extents,
	const std::vector<float> &coefficients,
	thread_pool &pool
)
{
	const auto describe = [](const std::vector<std::size_t> &extents)
	{
		return operand_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			numerical_type::float32,
			nullptr
		);
	};
	const auto fill = [](const std::vector<float> &data)
	{
		const auto result = make_float32_buffer(data.size());
		auto *host = static_cast<float*>(result->get_host_ptr());
		for (std::size_t i = 0; i < data.size(); ++i)
		{
			host[i] = data[i];
		}
		return result;
	};

	const Builder builder;
	const test_affine_operation operation;
	cpu_command_queue queue(get_serial_pool());
	const std::vector<operand_descriptor> outputs = {
		describe(image_extents)
	};
	const std::vector<operand_descriptor> inputs = {
		describe(image_extents),
		describe(coefficient_extents)
	};

	const auto program = builder.build(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue,
		nullptr
	);
	REQUIRE( program != nullptr );

	const auto result = make_float32_buffer(values.size());
	const std::vector<std::shared_ptr<buffer>> output_operands { result };
	const std::vector<std::shared_ptr<const buffer>> input_operands {
		fill(values),
		fill(coefficients)
	};
	const std::vector<std::shared_ptr<buffer>> scratch_operands;
	dynamic_cast<cpu_program&>(*program).execute(
		make_span(output_operands),
		make_span(input_operands),
		make_span(scratch_operands),
		pool
	);

	const auto *data = static_cast<const float*>(result->get_host_ptr());
	return std::vector<float>(data, data + values.size());
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

TEST_CASE(
	"elementwise_program_builder should make the kernel of every batch "
	"element from an input of its own core",
	"[elementwise_program_builder]"
)
{
	// Three images of 2 by 3, each with a scale and an offset.
	std::vector<float> values(18);
	for (std::size_t i = 0; i < values.size(); ++i)
	{
		values[i] = static_cast<float>(i);
	}
	const std::vector<float> coefficients = { 1, 100, 2, 200, 3, 300 };
	thread_pool threads(3);
	auto &pool = GENERATE(0, 1) == 0 ? *get_serial_pool() : threads;

	const auto result = run_affine<affine_builder<linear_index_tag>>(
		{ 3, 2, 3 },
		values,
		{ 3, 2 },
		coefficients,
		pool
	);

	REQUIRE( result.size() == 18 );
	for (std::size_t image = 0; image < 3; ++image)
	{
		for (std::size_t k = 0; k < 6; ++k)
		{
			INFO( "image " << image << ", element " << k );
			const auto i = 6 * image + k;
			CHECK(
				result[i] ==
				coefficients[2 * image] * values[i] +
					coefficients[2 * image + 1] + static_cast<float>(k)
			);
		}
	}
}

TEST_CASE(
	"elementwise_program_builder should hand the kernel of a batch element "
	"the coordinates within its core",
	"[elementwise_program_builder]"
)
{
	const std::vector<float> values(24, 1.0f);
	const std::vector<float> coefficients = { 2, 10, 3, 20 };

	const auto result = run_affine<affine_builder<multidimensional_index_tag>>(
		{ 2, 4, 3 },
		values,
		{ 2, 2 },
		coefficients,
		*get_serial_pool()
	);

	REQUIRE( result.size() == 24 );
	for (std::size_t image = 0; image < 2; ++image)
	{
		const auto base = coefficients[2 * image] +
			coefficients[2 * image + 1];
		for (std::size_t row = 0; row < 4; ++row)
		{
			for (std::size_t column = 0; column < 3; ++column)
			{
				INFO( "image " << image << ", " << row << ", " << column );
				CHECK(
					result[12 * image + 3 * row + column] ==
					base + static_cast<float>(10 * row + column)
				);
			}
		}
	}
}

TEST_CASE(
	"elementwise_program_builder should map a single image without batch "
	"axes",
	"[elementwise_program_builder]"
)
{
	const auto result = run_affine<affine_builder<linear_index_tag>>(
		{ 1, 3 },
		{ 1, 2, 3 },
		{ 2 },
		{ 2, 700 },
		*get_serial_pool()
	);

	CHECK( result == std::vector<float>{ 702, 705, 708 } );
}
