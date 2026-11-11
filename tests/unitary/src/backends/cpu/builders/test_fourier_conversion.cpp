// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/fourier_conversion.hpp>
#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/plans/fourier_layout_plan.hpp>

#include "../serial_pool.hpp"

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/fixed_width_float.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>

#include <array>
#include <complex>
#include <cstddef>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

using complex_type = std::complex<float64_t>;

operand_descriptor make_operand_descriptor(
	std::vector<std::size_t> extents,
	std::vector<std::ptrdiff_t> strides,
	numerical_type data_type
)
{
	return operand_descriptor(
		strided_layout::make_custom_layout(
			make_span(extents),
			make_span(strides),
			0
		),
		data_type,
		nullptr
	);
}

fourier_layout_plan make_plan(
	const operand_descriptor &output,
	const operand_descriptor &input
)
{
	const std::vector<std::size_t> axes = { 0 };
	return fourier_layout_plan(
		output,
		input,
		make_span(axes),
		fourier_transform_kind::complex_to_complex
	);
}

} // anonymous namespace

TEST_CASE(
	"run_fourier_conversion widens a real operand with no imaginary part",
	"[fourier_conversion]"
)
{
	const auto plan = make_plan(
		make_operand_descriptor({ 4 }, { 1 }, numerical_type::complex_float64),
		make_operand_descriptor({ 4 }, { 1 }, numerical_type::float64)
	);
	const std::array<float64_t, 4> input = {{ 1.0, 2.0, 3.0, 4.0 }};
	std::array<complex_type, 4> output{};

	run_fourier_conversion(
		plan,
		output.data(),
		input.data(),
		loop_schedule(*get_serial_pool(), 1)
	);

	for (std::size_t i = 0; i < 4; ++i)
	{
		CHECK( output[i] == complex_type(input[i], 0.0) );
	}
}

TEST_CASE(
	"run_fourier_conversion copies a complex operand across its strides",
	"[fourier_conversion]"
)
{
	const auto plan = make_plan(
		make_operand_descriptor({ 3 }, { 1 }, numerical_type::complex_float64),
		make_operand_descriptor({ 3 }, { 2 }, numerical_type::complex_float64)
	);
	const std::array<complex_type, 6> input = {{
		complex_type(1.0, -1.0),
		complex_type(9.0, 9.0),
		complex_type(2.0, -2.0),
		complex_type(9.0, 9.0),
		complex_type(3.0, -3.0),
		complex_type(9.0, 9.0)
	}};
	std::array<complex_type, 3> output{};

	run_fourier_conversion(
		plan,
		output.data(),
		input.data(),
		loop_schedule(*get_serial_pool(), 1)
	);

	CHECK( output[0] == complex_type(1.0, -1.0) );
	CHECK( output[1] == complex_type(2.0, -2.0) );
	CHECK( output[2] == complex_type(3.0, -3.0) );
}
