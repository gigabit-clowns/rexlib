// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/libraries/pocketfft/fft/pocketfft_fft.hpp>
#include <backends/cpu/skeletons/fourier/fourier_layout_plan.hpp>

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/fixed_width_float.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/ops/fourier/fourier_normalization.hpp>

#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

using complex_type = std::complex<float64_t>;

const std::array<complex_type, 4> ramp_spectrum = {{
	complex_type(10.0, 0.0),
	complex_type(-2.0, 2.0),
	complex_type(-2.0, 0.0),
	complex_type(-2.0, -2.0)
}};

operand_descriptor make_operand_descriptor(
	std::vector<std::size_t> extents,
	numerical_type data_type
)
{
	return operand_descriptor(
		strided_layout::make_contiguous_layout(make_span(extents)),
		data_type,
		nullptr
	);
}

fourier_layout_plan make_plan(
	const operand_descriptor &output,
	const operand_descriptor &input,
	std::vector<std::size_t> axes,
	fourier_transform_kind kind
)
{
	return fourier_layout_plan(output, input, make_span(axes), kind);
}

bool is_close(const complex_type &left, const complex_type &right)
{
	return std::abs(left - right) < 1e-12;
}

} // anonymous namespace

TEST_CASE(
	"pocketfft_fft::complex_to_complex transforms each row along the named "
	"axis",
	"[pocketfft_fft]"
)
{
	const auto descriptor =
		make_operand_descriptor({ 2, 4 }, numerical_type::complex_float64);
	const auto plan = make_plan(
		descriptor,
		descriptor,
		{ 1 },
		fourier_transform_kind::complex_to_complex
	);
	std::array<complex_type, 8> input{};
	for (std::size_t i = 0; i < 4; ++i)
	{
		input[i] = complex_type(static_cast<double>(i + 1), 0.0);
		input[4 + i] = complex_type(static_cast<double>(2 * (i + 1)), 0.0);
	}
	std::array<complex_type, 8> output{};

	pocketfft_fft::complex_to_complex(
		plan,
		ops::fourier_direction::forward,
		ops::fourier_normalization::ortho,
		output.data(),
		input.data(),
		1
	);

	// The ortho normalization divides by the square root of 4.
	for (std::size_t i = 0; i < 4; ++i)
	{
		INFO( "coefficient " << i );
		CHECK( is_close(output[i], ramp_spectrum[i] / 2.0) );
		CHECK( is_close(output[4 + i], ramp_spectrum[i]) );
	}
}

TEST_CASE(
	"pocketfft_fft::in_place_complex transforms the operand where it lies",
	"[pocketfft_fft]"
)
{
	const auto plan = make_plan(
		make_operand_descriptor({ 4 }, numerical_type::complex_float64),
		make_operand_descriptor({ 4 }, numerical_type::float64),
		{ 0 },
		fourier_transform_kind::complex_to_complex
	);
	std::array<complex_type, 4> data = ramp_spectrum;

	pocketfft_fft::in_place_complex(
		plan,
		ops::fourier_direction::inverse,
		ops::fourier_normalization::backward,
		data.data(),
		1
	);

	for (std::size_t i = 0; i < 4; ++i)
	{
		INFO( "sample " << i );
		CHECK(
			is_close(data[i], complex_type(static_cast<double>(i + 1), 0.0))
		);
	}
}

TEST_CASE(
	"pocketfft_fft::real_to_complex writes the half spectrum",
	"[pocketfft_fft]"
)
{
	const auto plan = make_plan(
		make_operand_descriptor({ 3 }, numerical_type::complex_float64),
		make_operand_descriptor({ 4 }, numerical_type::float64),
		{ 0 },
		fourier_transform_kind::real_to_complex
	);
	const std::array<float64_t, 4> input = {{ 1.0, 2.0, 3.0, 4.0 }};
	std::array<complex_type, 3> output{};

	pocketfft_fft::real_to_complex(
		plan,
		ops::fourier_direction::forward,
		ops::fourier_normalization::backward,
		output.data(),
		input.data(),
		1
	);

	for (std::size_t i = 0; i < 3; ++i)
	{
		INFO( "coefficient " << i );
		CHECK( is_close(output[i], ramp_spectrum[i]) );
	}
}

TEST_CASE(
	"pocketfft_fft::complex_to_real restores the real operand",
	"[pocketfft_fft]"
)
{
	const auto plan = make_plan(
		make_operand_descriptor({ 4 }, numerical_type::float64),
		make_operand_descriptor({ 3 }, numerical_type::complex_float64),
		{ 0 },
		fourier_transform_kind::complex_to_real
	);
	std::array<float64_t, 4> output{};

	// The forward normalization leaves the inverse transform unscaled.
	pocketfft_fft::complex_to_real(
		plan,
		ops::fourier_direction::inverse,
		ops::fourier_normalization::forward,
		output.data(),
		ramp_spectrum.data(),
		1
	);

	for (std::size_t i = 0; i < 4; ++i)
	{
		INFO( "sample " << i );
		CHECK( std::abs(output[i] - 4.0 * static_cast<double>(i + 1)) < 1e-12 );
	}
}
