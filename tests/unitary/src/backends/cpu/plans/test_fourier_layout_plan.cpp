// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/plans/fourier_layout_plan.hpp>

#include <rex/core/dispatch/operand_signature.hpp>
#include <rex/core/layout/joint_layout.hpp>
#include <rex/core/layout/strided_layout.hpp>
#include <rex/core/numerical/numerical_type.hpp>
#include <rex/core/span.hpp>

#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rex;
using namespace rex::cpu;

namespace
{

operand_signature make_signature(
	std::vector<std::size_t> extents,
	numerical_type data_type
)
{
	return operand_signature(
		strided_layout::make_contiguous_layout(make_span(extents)),
		data_type,
		nullptr
	);
}

operand_signature make_custom_signature(
	std::vector<std::size_t> extents,
	std::vector<std::ptrdiff_t> strides,
	std::ptrdiff_t offset,
	numerical_type data_type
)
{
	return operand_signature(
		strided_layout::make_custom_layout(
			make_span(extents),
			make_span(strides),
			offset
		),
		data_type,
		nullptr
	);
}

} // namespace

TEST_CASE(
	"fourier_layout_plan describes a transform between two complex operands",
	"[fourier_layout_plan]"
)
{
	const auto signature =
		make_signature({ 2, 4 }, numerical_type::complex_float32);
	const std::vector<std::size_t> transformed_axes = { 1 };

	const fourier_layout_plan plan(
		signature,
		signature,
		make_span(transformed_axes),
		fourier_transform_kind::complex_to_complex
	);

	CHECK( plan.get_shape() == std::vector<std::size_t>{ 2, 4 } );
	CHECK( plan.get_axes() == transformed_axes );
	CHECK( plan.get_sample_count() == 4 );
}

TEST_CASE(
	"fourier_layout_plan measures the strides in bytes",
	"[fourier_layout_plan]"
)
{
	// pocketfft counts in bytes so that it can transform an operand whose
	// elements are interleaved with something else, and a layout counts in
	// elements, so the element size is what stands between the two.
	const auto signature =
		make_signature({ 2, 4 }, numerical_type::complex_float64);
	const std::vector<std::size_t> transformed_axes = { 1 };

	const fourier_layout_plan plan(
		signature,
		signature,
		make_span(transformed_axes),
		fourier_transform_kind::complex_to_complex
	);

	const std::vector<std::ptrdiff_t> expected = { 4 * 16, 16 };
	CHECK( plan.get_input_strides() == expected );
	CHECK( plan.get_output_strides() == expected );
}

TEST_CASE(
	"fourier_layout_plan keeps the strides an operand actually has",
	"[fourier_layout_plan]"
)
{
	// A transform is not confined to operands laid out one after another, so
	// the strides are read off the layout rather than derived from the
	// extents.
	const auto input = make_custom_signature(
		{ 4 },
		{ 3 },
		5,
		numerical_type::complex_float32
	);
	const auto output = make_custom_signature(
		{ 4 },
		{ 2 },
		7,
		numerical_type::complex_float32
	);
	const std::vector<std::size_t> transformed_axes = { 0 };

	const fourier_layout_plan plan(
		output,
		input,
		make_span(transformed_axes),
		fourier_transform_kind::complex_to_complex
	);

	CHECK( plan.get_input_strides() == std::vector<std::ptrdiff_t>{ 3 * 8 } );
	CHECK( plan.get_output_strides() == std::vector<std::ptrdiff_t>{ 2 * 8 } );

	// The offsets belong to the pointers rather than to the traversal, and
	// are counted in elements because that is what a pointer is displaced by.
	CHECK( plan.get_input_offset() == 5 );
	CHECK( plan.get_output_offset() == 7 );
}

TEST_CASE(
	"fourier_layout_plan multiplies every transformed extent into the count",
	"[fourier_layout_plan]"
)
{
	const auto signature =
		make_signature({ 2, 3, 4 }, numerical_type::complex_float32);
	const std::vector<std::size_t> transformed_axes = { 1, 2 };

	const fourier_layout_plan plan(
		signature,
		signature,
		make_span(transformed_axes),
		fourier_transform_kind::complex_to_complex
	);

	// The batch axis is not one of them, an inverse transform undoing one
	// forward transform per batch element rather than one overall.
	CHECK( plan.get_sample_count() == 12 );
}

TEST_CASE(
	"fourier_layout_plan admits a transform along no axis at all",
	"[fourier_layout_plan]"
)
{
	const auto signature =
		make_signature({ 2, 4 }, numerical_type::complex_float32);
	const std::vector<std::size_t> transformed_axes;

	const fourier_layout_plan plan(
		signature,
		signature,
		make_span(transformed_axes),
		fourier_transform_kind::complex_to_complex
	);

	// Nothing is transformed and nothing is normalised by, so the result is
	// the operand itself.
	CHECK( plan.get_axes().empty() );
	CHECK( plan.get_sample_count() == 1 );
}

TEST_CASE(
	"fourier_layout_plan pairs the operands of a complex transform",
	"[fourier_layout_plan]"
)
{
	const auto signature =
		make_signature({ 2, 4 }, numerical_type::complex_float32);
	const std::vector<std::size_t> transformed_axes = { 1 };

	const fourier_layout_plan plan(
		signature,
		signature,
		make_span(transformed_axes),
		fourier_transform_kind::complex_to_complex
	);

	// Two operands of the same extents can be walked together, which is what
	// a real operand is widened through and what a transform along no axis
	// degenerates to.
	CHECK( plan.get_conversion_layout().get_rank() > 0 );
}

TEST_CASE(
	"fourier_layout_plan rejects two complex operands of different extents",
	"[fourier_layout_plan]"
)
{
	const auto output =
		make_signature({ 2, 3 }, numerical_type::complex_float32);
	const auto input =
		make_signature({ 2, 4 }, numerical_type::complex_float32);
	const std::vector<std::size_t> transformed_axes = { 1 };

	CHECK_THROWS_AS(
		fourier_layout_plan(
			output,
			input,
			make_span(transformed_axes),
			fourier_transform_kind::complex_to_complex
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"fourier_layout_plan halves the last transformed axis of a real transform",
	"[fourier_layout_plan]"
)
{
	const auto input = make_signature({ 3, 8 }, numerical_type::float32);
	const auto output =
		make_signature({ 3, 5 }, numerical_type::complex_float32);
	const std::vector<std::size_t> transformed_axes = { 1 };

	const fourier_layout_plan plan(
		output,
		input,
		make_span(transformed_axes),
		fourier_transform_kind::real_to_complex
	);

	// The shape is the signal's throughout: the halved extent follows from
	// it, and it is what the transform would be normalised by.
	CHECK( plan.get_shape() == std::vector<std::size_t>{ 3, 8 } );
	CHECK( plan.get_sample_count() == 8 );
}

TEST_CASE(
	"fourier_layout_plan halves only the last of several transformed axes",
	"[fourier_layout_plan]"
)
{
	// The symmetry of a real signal's spectrum relates a coefficient to the
	// one at the negated index along every transformed axis at once, so it
	// can only be exploited once.
	const auto input = make_signature({ 4, 6 }, numerical_type::float64);
	const auto output =
		make_signature({ 4, 4 }, numerical_type::complex_float64);
	const std::vector<std::size_t> transformed_axes = { 0, 1 };

	const fourier_layout_plan plan(
		output,
		input,
		make_span(transformed_axes),
		fourier_transform_kind::real_to_complex
	);

	CHECK( plan.get_shape() == std::vector<std::size_t>{ 4, 6 } );
	CHECK( plan.get_sample_count() == 24 );
}

TEST_CASE(
	"fourier_layout_plan halves an odd axis as an even one",
	"[fourier_layout_plan]"
)
{
	// Both 2*(m - 1) and 2*m - 1 samples store m coefficients, which is why
	// an inverse transform has to be told which of the two it came from.
	const auto input = make_signature({ 7 }, numerical_type::float32);
	const auto output = make_signature({ 4 }, numerical_type::complex_float32);
	const std::vector<std::size_t> transformed_axes = { 0 };

	const fourier_layout_plan plan(
		output,
		input,
		make_span(transformed_axes),
		fourier_transform_kind::real_to_complex
	);

	CHECK( plan.get_shape() == std::vector<std::size_t>{ 7 } );
	CHECK( plan.get_sample_count() == 7 );
}

TEST_CASE(
	"fourier_layout_plan rejects a wrongly sized half spectrum",
	"[fourier_layout_plan]"
)
{
	const auto input = make_signature({ 8 }, numerical_type::float32);
	const auto output = make_signature({ 8 }, numerical_type::complex_float32);
	const std::vector<std::size_t> transformed_axes = { 0 };

	CHECK_THROWS_AS(
		fourier_layout_plan(
			output,
			input,
			make_span(transformed_axes),
			fourier_transform_kind::real_to_complex
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"fourier_layout_plan takes the shape of an inverse real transform from "
	"its output",
	"[fourier_layout_plan]"
)
{
	// The signal is the output here, so the restored extent is what both the
	// halving and the normalisation are measured against.
	const auto input = make_signature({ 5 }, numerical_type::complex_float32);
	const auto output = make_signature({ 8 }, numerical_type::float32);
	const std::vector<std::size_t> transformed_axes = { 0 };

	const fourier_layout_plan plan(
		output,
		input,
		make_span(transformed_axes),
		fourier_transform_kind::complex_to_real
	);

	CHECK( plan.get_shape() == std::vector<std::size_t>{ 8 } );
	CHECK( plan.get_sample_count() == 8 );
}

TEST_CASE(
	"fourier_layout_plan restores an odd axis from the same coefficients",
	"[fourier_layout_plan]"
)
{
	const auto input = make_signature({ 5 }, numerical_type::complex_float32);
	const auto output = make_signature({ 9 }, numerical_type::float32);
	const std::vector<std::size_t> transformed_axes = { 0 };

	const fourier_layout_plan plan(
		output,
		input,
		make_span(transformed_axes),
		fourier_transform_kind::complex_to_real
	);

	CHECK( plan.get_shape() == std::vector<std::size_t>{ 9 } );
	CHECK( plan.get_sample_count() == 9 );
}

TEST_CASE(
	"fourier_layout_plan rejects a real transform along no axis",
	"[fourier_layout_plan]"
)
{
	// There is nothing to halve, so there is no telling what shape the
	// spectrum would have.
	const auto input = make_signature({ 8 }, numerical_type::float32);
	const auto output = make_signature({ 5 }, numerical_type::complex_float32);
	const std::vector<std::size_t> transformed_axes;

	CHECK_THROWS_AS(
		fourier_layout_plan(
			output,
			input,
			make_span(transformed_axes),
			fourier_transform_kind::real_to_complex
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"fourier_layout_plan rejects an axis outside the operand",
	"[fourier_layout_plan]"
)
{
	const auto signature =
		make_signature({ 2, 4 }, numerical_type::complex_float32);
	const std::vector<std::size_t> transformed_axes = { 2 };

	CHECK_THROWS_AS(
		fourier_layout_plan(
			signature,
			signature,
			make_span(transformed_axes),
			fourier_transform_kind::complex_to_complex
		),
		std::out_of_range
	);
}

TEST_CASE(
	"fourier_layout_plan rejects a transformed axis with no samples",
	"[fourier_layout_plan]"
)
{
	// There is nothing to read along it, and a half spectrum of an empty axis
	// still holds one coefficient, so there would be an output value with
	// nothing to put in it.
	const auto signature =
		make_signature({ 0, 4 }, numerical_type::complex_float32);
	const std::vector<std::size_t> transformed_axes = { 0 };

	CHECK_THROWS_AS(
		fourier_layout_plan(
			signature,
			signature,
			make_span(transformed_axes),
			fourier_transform_kind::complex_to_complex
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"fourier_layout_plan admits an empty axis it does not transform",
	"[fourier_layout_plan]"
)
{
	// A batch dimension with nothing in it asks for no transform rather than
	// for an impossible one.
	const auto signature =
		make_signature({ 0, 4 }, numerical_type::complex_float32);
	const std::vector<std::size_t> transformed_axes = { 1 };

	const fourier_layout_plan plan(
		signature,
		signature,
		make_span(transformed_axes),
		fourier_transform_kind::complex_to_complex
	);

	CHECK( plan.get_sample_count() == 4 );
}
