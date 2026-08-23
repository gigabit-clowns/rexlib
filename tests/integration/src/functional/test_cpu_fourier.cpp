// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rex/functional/fourier.hpp>

#include "fixtures/fourier_verb_fixture.hpp"

#include <rex/core/exceptions/invalid_operation_error.hpp>
#include <rex/ops/fourier/fourier_normalization.hpp>
#include <rex/core/numerical/fixed_width_float.hpp>

#include <complex>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

using namespace rex;
using rex::test::element_value;
using rex::test::fourier_verb_fixture;

namespace
{

// The transform of {1, 2, 3, 4}, which is short enough to be worked out by
// hand and long enough that every one of its coefficients differs.
const std::vector<element_value> ramp = { 1, 2, 3, 4 };
const std::vector<element_value> ramp_spectrum = {
	element_value(10.0),
	element_value(-2.0, 2.0),
	element_value(-2.0),
	element_value(-2.0, -2.0)
};

} // anonymous namespace

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"fft transforms a real signal into its full spectrum",
	"[array_fourier][cpu]"
)
{
	// A real operand is admitted and comes out complex, redundant halves and
	// all. Nothing about the shape says the signal was real, which is what
	// rfft exists to exploit.
	auto signal = make_operand<float32_t>({ 4 }, ramp);
	const const_array_ref signal_ref = signal;

	check_values<std::complex<float32_t>>(
		rex::fft(signal_ref, context, nullptr),
		{ 4 },
		ramp_spectrum
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"fft transforms a complex signal",
	"[array_fourier][cpu]"
)
{
	// One turn of the unit circle per sample, which is the single frequency
	// the transform must put all of its weight on.
	auto signal = make_operand<std::complex<float32_t>>(
		{ 4 },
		{
			element_value(1.0, 0.0),
			element_value(0.0, 1.0),
			element_value(-1.0, 0.0),
			element_value(0.0, -1.0)
		}
	);
	const const_array_ref signal_ref = signal;

	check_values<std::complex<float32_t>>(
		rex::fft(signal_ref, context, nullptr),
		{ 4 },
		{ 0, element_value(4.0), 0, 0 }
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"fft treats every axis it is not given as a batch dimension",
	"[array_fourier][cpu]"
)
{
	// The two rows are transformed independently, so a constant row comes out
	// as its own total and nothing else.
	auto signal = make_operand<float32_t>(
		{ 2, 4 },
		{ 1, 2, 3, 4, 1, 1, 1, 1 }
	);
	const const_array_ref signal_ref = signal;

	check_values<std::complex<float32_t>>(
		rex::fft(signal_ref, context, nullptr),
		{ 2, 4 },
		{
			element_value(10.0),
			element_value(-2.0, 2.0),
			element_value(-2.0),
			element_value(-2.0, -2.0),
			element_value(4.0),
			0,
			0,
			0
		}
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"fft transforms along an axis that is not the last",
	"[array_fourier][cpu]"
)
{
	// The transformed axis is then the one with the larger stride, so this is
	// what pins that the strides reach pocketfft the right way round.
	auto signal = make_operand<float32_t>(
		{ 4, 2 },
		{ 1, 0, 2, 0, 3, 0, 4, 0 }
	);
	const const_array_ref signal_ref = signal;

	const std::vector<std::ptrdiff_t> transformed_axes = { 0 };
	check_values<std::complex<float32_t>>(
		rex::fft(signal_ref, make_span(transformed_axes), context, nullptr),
		{ 4, 2 },
		{
			element_value(10.0), 0,
			element_value(-2.0, 2.0), 0,
			element_value(-2.0), 0,
			element_value(-2.0, -2.0), 0
		}
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"fft counts an axis from the end when it is negative",
	"[array_fourier][cpu]"
)
{
	auto signal = make_operand<float32_t>({ 4 }, ramp);
	const const_array_ref signal_ref = signal;

	const std::vector<std::ptrdiff_t> transformed_axes = { -1 };
	check_values<std::complex<float32_t>>(
		rex::fft(signal_ref, make_span(transformed_axes), context, nullptr),
		{ 4 },
		ramp_spectrum
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"fft2 transforms the last two axes together",
	"[array_fourier][cpu]"
)
{
	auto signal = make_operand<float32_t>({ 2, 2 }, { 1, 2, 3, 4 });
	const const_array_ref signal_ref = signal;

	check_values<std::complex<float32_t>>(
		rex::fft2(signal_ref, context, nullptr),
		{ 2, 2 },
		{
			element_value(10.0),
			element_value(-2.0),
			element_value(-4.0),
			0
		}
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"fft along no axis at all writes the operand out unchanged",
	"[array_fourier][cpu]"
)
{
	// A transform along nothing is the identity, and pocketfft has no way of
	// being asked for it: it writes an output once per axis, so with no axis
	// it would write none.
	auto signal = make_operand<float32_t>({ 4 }, ramp);
	const const_array_ref signal_ref = signal;

	const std::vector<std::ptrdiff_t> no_axes;
	check_values<std::complex<float32_t>>(
		rex::fft(signal_ref, make_span(no_axes), context, nullptr),
		{ 4 },
		ramp
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"ifft undoes fft",
	"[array_fourier][cpu]"
)
{
	auto spectrum = make_operand<std::complex<float32_t>>(
		{ 4 },
		ramp_spectrum
	);
	const const_array_ref spectrum_ref = spectrum;

	check_values<std::complex<float32_t>>(
		rex::ifft(spectrum_ref, context, nullptr),
		{ 4 },
		ramp
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"ifft divides by the number of samples",
	"[array_fourier][cpu]"
)
{
	// The whole of the difference between a transform and its inverse, beyond
	// the direction: without it the two would not undo one another. The zero
	// frequency of an inverse transform is therefore the mean rather than the
	// total.
	auto spectrum = make_operand<std::complex<float32_t>>(
		{ 4 },
		{ 2, 2, 2, 2 }
	);
	const const_array_ref spectrum_ref = spectrum;

	check_values<std::complex<float32_t>>(
		rex::ifft(spectrum_ref, context, nullptr),
		{ 4 },
		{ element_value(2.0), 0, 0, 0 }
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"rfft stores only the half a real signal needs",
	"[array_fourier][cpu]"
)
{
	// The other half is the conjugate of this one, so an axis of four samples
	// comes out with three coefficients rather than four.
	auto signal = make_operand<float32_t>({ 4 }, ramp);
	const const_array_ref signal_ref = signal;

	check_values<std::complex<float32_t>>(
		rex::rfft(signal_ref, context, nullptr),
		{ 3 },
		{ ramp_spectrum[0], ramp_spectrum[1], ramp_spectrum[2] }
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"rfft keeps every transformed axis but the last whole",
	"[array_fourier][cpu]"
)
{
	// The symmetry relates a coefficient to the one at the negated index
	// along every transformed axis at once, so it can only be exploited once
	// and only the last axis is halved.
	auto signal = make_operand<float32_t>(
		{ 2, 4 },
		{ 1, 2, 3, 4, 5, 6, 7, 8 }
	);
	const const_array_ref signal_ref = signal;

	check_values<std::complex<float32_t>>(
		rex::rfft2(signal_ref, context, nullptr),
		{ 2, 3 },
		{
			element_value(36.0),
			element_value(-4.0, 4.0),
			element_value(-4.0),
			element_value(-16.0),
			0,
			0
		}
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"irfft restores the signal rfft came from",
	"[array_fourier][cpu]"
)
{
	auto signal = make_operand<float32_t>({ 4 }, ramp);
	const const_array_ref signal_ref = signal;

	const auto spectrum = rex::rfft(signal_ref, context, nullptr);
	const const_array_ref spectrum_ref = spectrum;

	check_values<float32_t>(
		rex::irfft(spectrum_ref, 4, context, nullptr),
		{ 4 },
		ramp
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"irfft restores an odd signal from the same count of coefficients",
	"[array_fourier][cpu]"
)
{
	// Five samples store three coefficients and so do four, which is the one
	// thing an inverse real transform cannot work out for itself and the
	// reason the extent is named.
	const std::vector<element_value> odd_ramp = { 1, 2, 3, 4, 5 };

	auto signal = make_operand<float32_t>({ 5 }, odd_ramp);
	const const_array_ref signal_ref = signal;

	const auto spectrum = rex::rfft(signal_ref, context, nullptr);
	const const_array_ref spectrum_ref = spectrum;
	CHECK( spectrum.get_descriptor().get_layout().get_rank() == 1 );

	check_values<float32_t>(
		rex::irfft(spectrum_ref, 5, context, nullptr),
		{ 5 },
		odd_ramp
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"the transforms compute in double precision when the operand does",
	"[array_fourier][cpu]"
)
{
	auto signal = make_operand<float64_t>({ 4 }, ramp);
	const const_array_ref signal_ref = signal;

	check_values<std::complex<float64_t>>(
		rex::fft(signal_ref, context, nullptr),
		{ 4 },
		ramp_spectrum,
		1e-12
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"fft writes into an array it is given",
	"[array_fourier][cpu]"
)
{
	auto signal = make_operand<float32_t>({ 4 }, ramp);
	const const_array_ref signal_ref = signal;

	auto out = make_operand<std::complex<float32_t>>({ 4 }, { 0, 0, 0, 0 });
	const auto *out_storage = out.get_storage();

	const auto result = rex::fft(signal_ref, context, &out);

	CHECK( result.get_storage() == out_storage );
	check_values<std::complex<float32_t>>(result, { 4 }, ramp_spectrum);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"the transforms turn half precision away",
	"[array_fourier][cpu]"
)
{
	// A transform cannot be widened element by element the way an elementwise
	// kernel can, so this backend has no half precision transform to offer
	// and says so rather than pretending. The operation itself admits the
	// type, which is why this surfaces from dispatch rather than from typing.
	auto signal = make_operand<float16_t>({ 4 }, ramp);
	const const_array_ref signal_ref = signal;

	CHECK_THROWS_AS(
		rex::fft(signal_ref, context, nullptr),
		invalid_operation_error
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"the transforms reject an operand that cannot hold a fraction",
	"[array_fourier][cpu]"
)
{
	auto signal = make_operand<std::int32_t>({ 4 }, ramp);
	const const_array_ref signal_ref = signal;

	CHECK_THROWS_AS(
		rex::fft(signal_ref, context, nullptr),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"rfft rejects a complex operand",
	"[array_fourier][cpu]"
)
{
	// A complex signal's spectrum has no symmetry to exploit, so storing half
	// of one would lose information.
	auto signal = make_operand<std::complex<float32_t>>({ 4 }, ramp);
	const const_array_ref signal_ref = signal;

	CHECK_THROWS_AS(
		rex::rfft(signal_ref, context, nullptr),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"the transforms turn away an axis with no samples",
	"[array_fourier][cpu]"
)
{
	// The shape of a spectrum does not go empty with the signal: a half
	// spectrum of an empty axis still holds one coefficient, and there is
	// nothing that could be put in it.
	auto signal = make_operand<float32_t>({ 0 }, {});
	const const_array_ref signal_ref = signal;

	CHECK_THROWS_AS(
		rex::fft(signal_ref, context, nullptr),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"the transforms admit an empty axis they do not transform",
	"[array_fourier][cpu]"
)
{
	// A batch dimension with nothing in it asks for no transform rather than
	// for an impossible one, so the result is an array of the same shape with
	// nothing in it either.
	auto signal = make_operand<float32_t>({ 0, 4 }, {});
	const const_array_ref signal_ref = signal;

	check_values<std::complex<float32_t>>(
		rex::fft(signal_ref, context, nullptr),
		{ 0, 4 },
		{}
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"fft along no axis writes out an operand that has none either",
	"[array_fourier][cpu]"
)
{
	// A scalar has no axis to transform and cannot be given one, so the only
	// transform it admits is the identity.
	auto signal = make_operand<float32_t>({}, { element_value(3.0) });
	const const_array_ref signal_ref = signal;

	const std::vector<std::ptrdiff_t> no_axes;
	check_values<std::complex<float32_t>>(
		rex::fft(signal_ref, make_span(no_axes), context, nullptr),
		{},
		{ element_value(3.0) }
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"fft splits the scaling with its inverse when asked for an orthonormal "
	"transform",
	"[array_fourier][cpu]"
)
{
	// Four samples, so each transform of the pair carries a half rather than
	// one of them carrying a quarter.
	auto signal = make_operand<float32_t>({ 4 }, ramp);
	const const_array_ref signal_ref = signal;

	check_values<std::complex<float32_t>>(
		rex::fft(
			signal_ref,
			ops::fourier_normalization::ortho,
			context,
			nullptr
		),
		{ 4 },
		{
			element_value(5.0),
			element_value(-1.0, 1.0),
			element_value(-1.0),
			element_value(-1.0, -1.0)
		}
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"fft carries the whole of the scaling when asked to",
	"[array_fourier][cpu]"
)
{
	// The zero frequency is then the mean of the signal rather than its
	// total, which is the whole point of moving the scaling forwards.
	auto signal = make_operand<float32_t>({ 4 }, ramp);
	const const_array_ref signal_ref = signal;

	check_values<std::complex<float32_t>>(
		rex::fft(
			signal_ref,
			ops::fourier_normalization::forward,
			context,
			nullptr
		),
		{ 4 },
		{
			element_value(2.5),
			element_value(-0.5, 0.5),
			element_value(-0.5),
			element_value(-0.5, -0.5)
		}
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"ifft leaves the result alone when the forward transform carried the "
	"scaling",
	"[array_fourier][cpu]"
)
{
	auto spectrum = make_operand<std::complex<float32_t>>(
		{ 4 },
		ramp_spectrum
	);
	const const_array_ref spectrum_ref = spectrum;

	check_values<std::complex<float32_t>>(
		rex::ifft(
			spectrum_ref,
			ops::fourier_normalization::forward,
			context,
			nullptr
		),
		{ 4 },
		{ 4, 8, 12, 16 }
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"a transform and its inverse undo one another under every convention",
	"[array_fourier][cpu]"
)
{
	// The property the convention is named for rather than the factor. The
	// caller states the same thing on both sides and gets its signal back,
	// with no pairing up of two different factors to get wrong.
	const auto normalization = GENERATE(
		ops::fourier_normalization::backward,
		ops::fourier_normalization::ortho,
		ops::fourier_normalization::forward
	);
	INFO( "convention " << normalization );

	auto signal = make_operand<std::complex<float32_t>>({ 4 }, ramp);
	const const_array_ref signal_ref = signal;

	const auto spectrum =
		rex::fft(signal_ref, normalization, context, nullptr);
	const const_array_ref spectrum_ref = spectrum;

	check_values<std::complex<float32_t>>(
		rex::ifft(spectrum_ref, normalization, context, nullptr),
		{ 4 },
		ramp
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"a real transform and its inverse undo one another under every "
	"convention",
	"[array_fourier][cpu]"
)
{
	// The scaling of a real transform is measured against the signal rather
	// than against the coefficients actually stored, which is what keeps this
	// true when half of them are not there.
	const auto normalization = GENERATE(
		ops::fourier_normalization::backward,
		ops::fourier_normalization::ortho,
		ops::fourier_normalization::forward
	);
	INFO( "convention " << normalization );

	auto signal = make_operand<float32_t>({ 4 }, ramp);
	const const_array_ref signal_ref = signal;

	const auto spectrum =
		rex::rfft(signal_ref, normalization, context, nullptr);
	const const_array_ref spectrum_ref = spectrum;

	check_values<float32_t>(
		rex::irfft(spectrum_ref, 4, normalization, context, nullptr),
		{ 4 },
		ramp
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"rfft scales against the signal rather than against what it stores",
	"[array_fourier][cpu]"
)
{
	// Four samples come out as three coefficients, and it is the four that
	// the orthonormal square root is taken of.
	auto signal = make_operand<float32_t>({ 4 }, ramp);
	const const_array_ref signal_ref = signal;

	check_values<std::complex<float32_t>>(
		rex::rfft(
			signal_ref,
			ops::fourier_normalization::ortho,
			context,
			nullptr
		),
		{ 3 },
		{
			element_value(5.0),
			element_value(-1.0, 1.0),
			element_value(-1.0)
		}
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"a numbered transform counts every axis it transforms into the scaling",
	"[array_fourier][cpu]"
)
{
	// Two axes of two, so the orthonormal factor is the square root of four
	// rather than of two: the count is over the whole transformed subspace.
	auto signal = make_operand<float32_t>({ 2, 2 }, { 1, 2, 3, 4 });
	const const_array_ref signal_ref = signal;

	check_values<std::complex<float32_t>>(
		rex::fft2(
			signal_ref,
			ops::fourier_normalization::ortho,
			context,
			nullptr
		),
		{ 2, 2 },
		{
			element_value(5.0),
			element_value(-1.0),
			element_value(-2.0),
			0
		}
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"a transform told no convention scales as the default one does",
	"[array_fourier][cpu]"
)
{
	// Pins which of the three the shorter spelling picked, so that it cannot
	// drift without a test saying so.
	auto signal = make_operand<float32_t>({ 4 }, ramp);
	const const_array_ref signal_ref = signal;

	check_values<std::complex<float32_t>>(
		rex::fft(
			signal_ref,
			ops::fourier_normalization::backward,
			context,
			nullptr
		),
		{ 4 },
		ramp_spectrum
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"fftshift moves the zero frequency to the centre",
	"[array_fourier][cpu]"
)
{
	// Odd extent: fftshift and ifftshift disagree by one element, which is
	// exactly what distinguishes the two.
	auto signal = make_operand<float32_t>({ 5 }, { 0, 1, 2, 3, 4 });
	const const_array_ref signal_ref = signal;

	check_values<float32_t>(
		rex::fftshift(signal_ref, context, nullptr),
		{ 5 },
		{ 3, 4, 0, 1, 2 }
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"ifftshift undoes fftshift",
	"[array_fourier][cpu]"
)
{
	auto shifted = make_operand<float32_t>({ 5 }, { 3, 4, 0, 1, 2 });
	const const_array_ref shifted_ref = shifted;

	check_values<float32_t>(
		rex::ifftshift(shifted_ref, context, nullptr),
		{ 5 },
		{ 0, 1, 2, 3, 4 }
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"fftshift with an even extent shifts by half",
	"[array_fourier][cpu]"
)
{
	auto signal = make_operand<float32_t>({ 4 }, { 0, 1, 2, 3 });
	const const_array_ref signal_ref = signal;

	check_values<float32_t>(
		rex::fftshift(signal_ref, context, nullptr),
		{ 4 },
		{ 2, 3, 0, 1 }
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"fftshift along every axis shifts each one independently",
	"[array_fourier][cpu]"
)
{
	// Exercises the block decomposition with more than one shifted axis:
	// rows and columns wrap on their own, giving 2^2 = 4 blocks.
	auto signal = make_operand<float32_t>(
		{ 2, 3 },
		{ 0, 1, 2, 3, 4, 5 }
	);
	const const_array_ref signal_ref = signal;

	check_values<float32_t>(
		rex::fftshift(signal_ref, context, nullptr),
		{ 2, 3 },
		{ 5, 3, 4, 2, 0, 1 }
	);
}

TEST_CASE_METHOD(
	fourier_verb_fixture,
	"fftshift along a single axis leaves the other untouched",
	"[array_fourier][cpu]"
)
{
	auto signal = make_operand<float32_t>(
		{ 2, 3 },
		{ 0, 1, 2, 3, 4, 5 }
	);
	const const_array_ref signal_ref = signal;

	const std::vector<std::ptrdiff_t> axes = { 1 };
	check_values<float32_t>(
		rex::fftshift(signal_ref, make_span(axes), context, nullptr),
		{ 2, 3 },
		{ 2, 0, 1, 5, 3, 4 }
	);
}
