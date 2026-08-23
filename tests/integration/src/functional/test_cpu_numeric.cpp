// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/numeric.hpp>

#include "fixtures/elementwise_verb_fixture.hpp"

#include <rex/ops/numeric/copysign_operation.hpp>
#include <rex/ops/numeric/is_finite_operation.hpp>
#include <rex/ops/numeric/is_inf_operation.hpp>
#include <rex/ops/numeric/is_nan_operation.hpp>
#include <rex/ops/numeric/sign_bit_operation.hpp>

#include <limits>

using namespace rex;
using namespace rex::ops;
using xmipp4::test::element_value;
using xmipp4::test::elementwise_verb_fixture;

namespace
{

const double not_a_number = std::numeric_limits<double>::quiet_NaN();
const double infinity = std::numeric_limits<double>::infinity();

} // anonymous namespace

// These report on the representation of a number rather than on its
// magnitude, so the cases are built from the values that only a
// representation distinguishes: the infinities, the not-a-numbers and the
// negative zero.

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"is_nan holds for an element that is not a number",
	"[array_numeric][cpu]"
)
{
	// A complex operand takes it as its real part, and a complex number
	// with such a part is not a number either, so one case covers both.
	check_unary<is_nan_operation>(
		xmipp4::is_nan,
		element_value(not_a_number),
		[](auto) { return true; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"is_nan fails for an ordinary element",
	"[array_numeric][cpu]"
)
{
	check_unary<is_nan_operation>(
		xmipp4::is_nan,
		element_value(1),
		[](auto) { return false; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"is_nan holds when only the imaginary part is not a number",
	"[array_numeric][cpu]"
)
{
	check_unary<is_nan_operation, complex_type_domain>(
		xmipp4::is_nan,
		element_value(0, not_a_number),
		[](auto) { return true; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"is_inf holds for an infinite element",
	"[array_numeric][cpu]"
)
{
	check_unary<is_inf_operation>(
		xmipp4::is_inf,
		element_value(infinity),
		[](auto) { return true; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"is_inf fails for an element that is not a number",
	"[array_numeric][cpu]"
)
{
	// Neither finite nor infinite, which is what keeps is_inf from being
	// the negation of is_finite.
	check_unary<is_inf_operation>(
		xmipp4::is_inf,
		element_value(not_a_number),
		[](auto) { return false; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"is_finite holds for an ordinary element",
	"[array_numeric][cpu]"
)
{
	check_unary<is_finite_operation>(
		xmipp4::is_finite,
		element_value(1),
		[](auto) { return true; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"is_finite fails for an infinite element",
	"[array_numeric][cpu]"
)
{
	check_unary<is_finite_operation>(
		xmipp4::is_finite,
		element_value(infinity),
		[](auto) { return false; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"is_finite fails when only the imaginary part is infinite",
	"[array_numeric][cpu]"
)
{
	// Both parts must be finite, unlike is_nan and is_inf, which need
	// only one part to answer.
	check_unary<is_finite_operation, complex_type_domain>(
		xmipp4::is_finite,
		element_value(0, infinity),
		[](auto) { return false; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"sign_bit holds for a negative element",
	"[array_numeric][cpu]"
)
{
	check_unary<sign_bit_operation>(
		xmipp4::sign_bit,
		element_value(-1),
		[](auto) { return true; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"sign_bit holds for a negative zero",
	"[array_numeric][cpu]"
)
{
	// A negative zero compares equal to a positive one, so this is what
	// separates sign_bit from a comparison against zero.
	check_unary<sign_bit_operation>(
		xmipp4::sign_bit,
		element_value(-0.0),
		[](auto) { return true; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"sign_bit fails for a positive zero",
	"[array_numeric][cpu]"
)
{
	check_unary<sign_bit_operation>(
		xmipp4::sign_bit,
		element_value(0.0),
		[](auto) { return false; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"copysign takes the magnitude of the first element and the sign of "
	"the second",
	"[array_numeric][cpu]"
)
{
	check_binary<copysign_operation>(
		xmipp4::copysign,
		element_value(3),
		element_value(-1),
		[](auto, auto) { return -3; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"copysign discards the sign the magnitude already had",
	"[array_numeric][cpu]"
)
{
	// Taken the other way round this would be -3, so the case pins which
	// operand each role belongs to.
	check_binary<copysign_operation>(
		xmipp4::copysign,
		element_value(-3),
		element_value(1),
		[](auto, auto) { return 3; }
	);
}
