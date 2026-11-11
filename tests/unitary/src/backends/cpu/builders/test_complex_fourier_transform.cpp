// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/complex_fourier_transform.hpp>
#include <backends/cpu/kernels/pocketfft_fft.hpp>
#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/plans/fourier_layout_plan.hpp>

#include "../serial_pool.hpp"

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/fixed_width_float.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/ops/fourier/fourier_normalization.hpp>

#include <array>
#include <complex>
#include <cstddef>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

using complex_type = std::complex<float64_t>;

using forward_transform = forward_complex_fourier_transform<pocketfft_fft>;
using inverse_transform = inverse_complex_fourier_transform<pocketfft_fft>;

const std::array<complex_type, 4> ramp_spectrum = {{
	complex_type(10.0, 0.0),
	complex_type(-2.0, 2.0),
	complex_type(-2.0, 0.0),
	complex_type(-2.0, -2.0)
}};

operand_descriptor make_operand_descriptor(numerical_type data_type)
{
	const std::vector<std::size_t> extents = { 4 };
	return operand_descriptor(
		strided_layout::make_contiguous_layout(make_span(extents)),
		data_type,
		nullptr
	);
}

fourier_layout_plan make_plan(
	numerical_type input_type,
	std::vector<std::size_t> axes
)
{
	return fourier_layout_plan(
		make_operand_descriptor(numerical_type::complex_float64),
		make_operand_descriptor(input_type),
		make_span(axes),
		fourier_transform_kind::complex_to_complex
	);
}

loop_schedule make_schedule()
{
	return loop_schedule(*get_serial_pool(), 1);
}

bool is_close(const complex_type &left, const complex_type &right)
{
	return std::abs(left - right) < 1e-12;
}

} // anonymous namespace

TEST_CASE(
	"complex_fourier_transform reports a complex to complex transform",
	"[complex_fourier_transform]"
)
{
	CHECK(
		forward_transform::get_kind() ==
		fourier_transform_kind::complex_to_complex
	);
	CHECK(
		inverse_transform::get_kind() ==
		fourier_transform_kind::complex_to_complex
	);
}

TEST_CASE(
	"complex_fourier_transform transforms a complex operand forward",
	"[complex_fourier_transform]"
)
{
	const forward_transform transform(ops::fourier_normalization::backward);
	const auto plan = make_plan(numerical_type::complex_float64, { 0 });
	const std::array<complex_type, 4> input = {{
		complex_type(1.0, 0.0),
		complex_type(2.0, 0.0),
		complex_type(3.0, 0.0),
		complex_type(4.0, 0.0)
	}};
	std::array<complex_type, 4> output{};

	transform(plan, output.data(), input.data(), make_schedule());

	for (std::size_t i = 0; i < 4; ++i)
	{
		INFO( "coefficient " << i );
		CHECK( is_close(output[i], ramp_spectrum[i]) );
	}
}

TEST_CASE(
	"complex_fourier_transform widens a real operand before transforming",
	"[complex_fourier_transform]"
)
{
	const forward_transform transform(ops::fourier_normalization::backward);
	const auto plan = make_plan(numerical_type::float64, { 0 });
	const std::array<float64_t, 4> input = {{ 1.0, 2.0, 3.0, 4.0 }};
	std::array<complex_type, 4> output{};

	transform(plan, output.data(), input.data(), make_schedule());

	for (std::size_t i = 0; i < 4; ++i)
	{
		INFO( "coefficient " << i );
		CHECK( is_close(output[i], ramp_spectrum[i]) );
	}
}

TEST_CASE(
	"complex_fourier_transform scales the inverse by its normalization",
	"[complex_fourier_transform]"
)
{
	const inverse_transform transform(ops::fourier_normalization::backward);
	const auto plan = make_plan(numerical_type::complex_float64, { 0 });
	std::array<complex_type, 4> output{};

	transform(plan, output.data(), ramp_spectrum.data(), make_schedule());

	for (std::size_t i = 0; i < 4; ++i)
	{
		INFO( "sample " << i );
		CHECK(
			is_close(output[i], complex_type(static_cast<double>(i + 1), 0.0))
		);
	}
}

TEST_CASE(
	"complex_fourier_transform along no axis copies the operand across",
	"[complex_fourier_transform]"
)
{
	const forward_transform transform(ops::fourier_normalization::forward);

	SECTION("complex operand")
	{
		const auto plan = make_plan(numerical_type::complex_float64, {});
		std::array<complex_type, 4> output{};

		transform(plan, output.data(), ramp_spectrum.data(), make_schedule());

		CHECK( output == ramp_spectrum );
	}

	SECTION("real operand")
	{
		const auto plan = make_plan(numerical_type::float64, {});
		const std::array<float64_t, 4> input = {{ 1.0, 2.0, 3.0, 4.0 }};
		std::array<complex_type, 4> output{};

		transform(plan, output.data(), input.data(), make_schedule());

		for (std::size_t i = 0; i < 4; ++i)
		{
			CHECK( output[i] == complex_type(input[i], 0.0) );
		}
	}
}
