// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/trigonometric.hpp>

#include "fixtures/elementwise_verb_fixture.hpp"

#include <rex/ops/trigonometric/acos_operation.hpp>
#include <rex/ops/trigonometric/acosh_operation.hpp>
#include <rex/ops/trigonometric/asin_operation.hpp>
#include <rex/ops/trigonometric/asinh_operation.hpp>
#include <rex/ops/trigonometric/atan2_operation.hpp>
#include <rex/ops/trigonometric/atan_operation.hpp>
#include <rex/ops/trigonometric/atanh_operation.hpp>
#include <rex/ops/trigonometric/cos_operation.hpp>
#include <rex/ops/trigonometric/cosh_operation.hpp>
#include <rex/ops/trigonometric/degrees_operation.hpp>
#include <rex/ops/trigonometric/hypot_operation.hpp>
#include <rex/ops/trigonometric/radians_operation.hpp>
#include <rex/ops/trigonometric/sin_operation.hpp>
#include <rex/ops/trigonometric/sincos_operation.hpp>
#include <rex/ops/trigonometric/sinh_operation.hpp>
#include <rex/ops/trigonometric/tan_operation.hpp>
#include <rex/ops/trigonometric/tanh_operation.hpp>

using namespace rex;
using namespace rex::ops;
using xmipp4::test::comparison_mode;
using xmipp4::test::element_value;
using xmipp4::test::elementwise_verb_fixture;

// None of these has a correctly rounded result guaranteed by IEEE-754, so
// they are compared within a tolerance for their element type. The inputs
// are real, which for a complex operand means a point on the real axis and
// therefore a result whose imaginary part is zero.
//
// Every verb is named with its namespace: <cmath> puts a function of the
// same name in the global one, and naming a verb rather than calling it
// cannot be resolved by its arguments.

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"sin takes the sine of each element",
	"[array_trigonometric][cpu]"
)
{
	check_unary<sin_operation>(
		xmipp4::sin,
		element_value(1),
		[](auto) { return 0.8414709848078965; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"cos takes the cosine of each element",
	"[array_trigonometric][cpu]"
)
{
	check_unary<cos_operation>(
		xmipp4::cos,
		element_value(1),
		[](auto) { return 0.5403023058681398; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"tan takes the tangent of each element",
	"[array_trigonometric][cpu]"
)
{
	check_unary<tan_operation>(
		xmipp4::tan,
		element_value(1),
		[](auto) { return 1.5574077246549023; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"asin takes the arc sine of each element",
	"[array_trigonometric][cpu]"
)
{
	check_unary<asin_operation>(
		xmipp4::asin,
		element_value(0.5),
		[](auto) { return 0.5235987755982989; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"acos takes the arc cosine of each element",
	"[array_trigonometric][cpu]"
)
{
	check_unary<acos_operation>(
		xmipp4::acos,
		element_value(0.5),
		[](auto) { return 1.0471975511965979; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"atan takes the arc tangent of each element",
	"[array_trigonometric][cpu]"
)
{
	check_unary<atan_operation>(
		xmipp4::atan,
		element_value(1),
		[](auto) { return 0.7853981633974483; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"sinh takes the hyperbolic sine of each element",
	"[array_trigonometric][cpu]"
)
{
	check_unary<sinh_operation>(
		xmipp4::sinh,
		element_value(1),
		[](auto) { return 1.1752011936438014; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"cosh takes the hyperbolic cosine of each element",
	"[array_trigonometric][cpu]"
)
{
	check_unary<cosh_operation>(
		xmipp4::cosh,
		element_value(1),
		[](auto) { return 1.5430806348152437; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"tanh takes the hyperbolic tangent of each element",
	"[array_trigonometric][cpu]"
)
{
	check_unary<tanh_operation>(
		xmipp4::tanh,
		element_value(1),
		[](auto) { return 0.7615941559557649; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"asinh takes the inverse hyperbolic sine of each element",
	"[array_trigonometric][cpu]"
)
{
	check_unary<asinh_operation>(
		xmipp4::asinh,
		element_value(1),
		[](auto) { return 0.881373587019543; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"acosh takes the inverse hyperbolic cosine of each element",
	"[array_trigonometric][cpu]"
)
{
	check_unary<acosh_operation>(
		xmipp4::acosh,
		element_value(2),
		[](auto) { return 1.3169578969248166; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"atanh takes the inverse hyperbolic tangent of each element",
	"[array_trigonometric][cpu]"
)
{
	check_unary<atanh_operation>(
		xmipp4::atanh,
		element_value(0.5),
		[](auto) { return 0.5493061443340549; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"atan2 takes the ordinate first, so a zero abscissa gives a right angle",
	"[array_trigonometric][cpu]"
)
{
	// Deliberately asymmetric: were the operands taken the other way round
	// this would be zero rather than a quarter turn.
	check_binary<atan2_operation>(
		xmipp4::atan2,
		element_value(1),
		element_value(0),
		[](auto, auto) { return 1.5707963267948966; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"hypot takes the length of the hypotenuse of each pair",
	"[array_trigonometric][cpu]"
)
{
	check_binary<hypot_operation>(
		xmipp4::hypot,
		element_value(3),
		element_value(4),
		[](auto, auto) { return 5; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"degrees converts each element out of radians",
	"[array_trigonometric][cpu]"
)
{
	check_unary<degrees_operation>(
		xmipp4::degrees,
		element_value(1),
		[](auto) { return 57.29577951308232; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"radians converts each element into radians",
	"[array_trigonometric][cpu]"
)
{
	check_unary<radians_operation>(
		xmipp4::radians,
		element_value(180),
		[](auto) { return 3.141592653589793; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"sincos gives the sine and the cosine of each element in that order",
	"[array_trigonometric][cpu]"
)
{
	check_unary_pair<sincos_operation>(
		xmipp4::sincos,
		element_value(1),
		[](auto) { return 0.8414709848078965; },
		[](auto) { return 0.5403023058681398; },
		comparison_mode::approximate
	);
}
