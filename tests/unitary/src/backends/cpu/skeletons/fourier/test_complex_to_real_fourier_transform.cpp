// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/skeletons/fourier/complex_to_real_fourier_transform.hpp>
#include <backends/cpu/pocketfft/fft/pocketfft_fft.hpp>
#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/skeletons/fourier/fourier_layout_plan.hpp>

#include "../../hardware/mock/serial_pool.hpp"

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

using transform_type = complex_to_real_fourier_transform<pocketfft_fft>;

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
	"complex_to_real_fourier_transform reports a complex to real transform",
	"[complex_to_real_fourier_transform]"
)
{
	CHECK(
		transform_type::get_kind() == fourier_transform_kind::complex_to_real
	);
}

TEST_CASE(
	"complex_to_real_fourier_transform transforms a half spectrum back into "
	"a real operand",
	"[complex_to_real_fourier_transform]"
)
{
	const transform_type transform(ops::fourier_normalization::backward);
	const std::vector<std::size_t> axes = { 0 };
	const fourier_layout_plan plan(
		make_operand_descriptor(4, numerical_type::float64),
		make_operand_descriptor(3, numerical_type::complex_float64),
		make_span(axes),
		fourier_transform_kind::complex_to_real
	);
	const std::array<complex_type, 3> input = {{
		complex_type(10.0, 0.0),
		complex_type(-2.0, 2.0),
		complex_type(-2.0, 0.0)
	}};
	std::array<float64_t, 4> output{};

	transform(
		plan,
		output.data(),
		input.data(),
		loop_schedule(*get_serial_pool(), 1)
	);

	for (std::size_t i = 0; i < 4; ++i)
	{
		INFO( "sample " << i );
		CHECK( std::abs(output[i] - static_cast<double>(i + 1)) < 1e-12 );
	}
}
