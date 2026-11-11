// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/real_to_complex_fourier_transform.hpp>
#include <backends/cpu/kernels/pocketfft_fft.hpp>
#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/plans/fourier_layout_plan.hpp>

#include "../hardware/mock/serial_pool.hpp"

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

using transform_type = real_to_complex_fourier_transform<pocketfft_fft>;

operand_descriptor make_operand_descriptor(
	std::size_t extent,
	numerical_type data_type
)
{
	const std::vector<std::size_t> extents = { extent };
	return operand_descriptor(
		strided_layout::make_contiguous_layout(make_span(extents)),
		data_type,
		nullptr
	);
}

} // anonymous namespace

TEST_CASE(
	"real_to_complex_fourier_transform reports a real to complex transform",
	"[real_to_complex_fourier_transform]"
)
{
	CHECK(
		transform_type::get_kind() == fourier_transform_kind::real_to_complex
	);
}

TEST_CASE(
	"real_to_complex_fourier_transform transforms a real operand forward "
	"into a half spectrum",
	"[real_to_complex_fourier_transform]"
)
{
	const transform_type transform(ops::fourier_normalization::forward);
	const std::vector<std::size_t> axes = { 0 };
	const fourier_layout_plan plan(
		make_operand_descriptor(3, numerical_type::complex_float64),
		make_operand_descriptor(4, numerical_type::float64),
		make_span(axes),
		fourier_transform_kind::real_to_complex
	);
	const std::array<float64_t, 4> input = {{ 1.0, 2.0, 3.0, 4.0 }};
	std::array<complex_type, 3> output{};

	transform(
		plan,
		output.data(),
		input.data(),
		loop_schedule(*get_serial_pool(), 1)
	);

	// The forward normalization divides the forward transform by 4.
	const std::array<complex_type, 3> expected = {{
		complex_type(2.5, 0.0),
		complex_type(-0.5, 0.5),
		complex_type(-0.5, 0.0)
	}};
	for (std::size_t i = 0; i < 3; ++i)
	{
		INFO( "coefficient " << i );
		CHECK( std::abs(output[i] - expected[i]) < 1e-12 );
	}
}
