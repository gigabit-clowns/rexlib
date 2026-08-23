// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/complex.hpp>

#include "fixtures/elementwise_verb_fixture.hpp"

#include <rex/ops/complex/angle_operation.hpp>
#include <rex/ops/complex/conjugate_operation.hpp>

#include <complex>

using namespace rex;
using namespace rex::ops;
using rex::test::comparison_mode;
using rex::test::element_value;
using rex::test::elementwise_verb_fixture;

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"angle of an element on the positive real axis is zero",
	"[array_complex][cpu]"
)
{
	// A real operand is the point on the real axis, so the real and the
	// complex types answer alike and one case covers both.
	check_unary<angle_operation>(
		rex::angle,
		element_value(1),
		[](auto) { return 0; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"angle of an element on the negative real axis is half a turn",
	"[array_complex][cpu]"
)
{
	check_unary<angle_operation>(
		rex::angle,
		element_value(-1),
		[](auto) { return 3.141592653589793; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"angle of an element off the real axis is its argument",
	"[array_complex][cpu]"
)
{
	// Straight up from the origin, a quarter turn. Only the complex types
	// can hold such an operand.
	check_unary<angle_operation, complex_type_domain>(
		rex::angle,
		element_value(0, 1),
		[](auto) { return 1.5707963267948966; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"angle produces the real counterpart of a complex element type",
	"[array_complex][cpu]"
)
{
	// The result type is asserted by the driver, which reifies it from
	// the operation's rule: complex_float32 in, float32 out.
	const auto descriptor =
		make_descriptor({ 2, 3 }, numerical_type::complex_float32);
	auto x = full(
		descriptor, memory_resource_affinity::device,
		scalar_value(std::complex<float>(0, 1)), context
	);
	const_array_ref x_ref = x;

	const auto result = rex::angle(x_ref, context, nullptr);

	CHECK( result.get_descriptor().get_data_type() == numerical_type::float32 );
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"conjugate leaves a real element alone",
	"[array_complex][cpu]"
)
{
	check_unary<conjugate_operation, real_arithmetic_type_domain>(
		rex::conjugate,
		element_value(2),
		[](auto x) { return x; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"conjugate flips the sign of an imaginary part",
	"[array_complex][cpu]"
)
{
	check_unary<conjugate_operation, complex_type_domain>(
		rex::conjugate,
		element_value(2, 3),
		[](auto x) { return std::conj(x); }
	);
}
