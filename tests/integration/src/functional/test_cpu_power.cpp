// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/power.hpp>

#include "fixtures/elementwise_verb_fixture.hpp"

#include <rex/ops/power/cbrt_operation.hpp>
#include <rex/ops/power/exp2_operation.hpp>
#include <rex/ops/power/exp_operation.hpp>
#include <rex/ops/power/expm1_operation.hpp>
#include <rex/ops/power/log10_operation.hpp>
#include <rex/ops/power/log1p_operation.hpp>
#include <rex/ops/power/log2_operation.hpp>
#include <rex/ops/power/log_operation.hpp>
#include <rex/ops/power/power_operation.hpp>
#include <rex/ops/power/reciprocal_operation.hpp>
#include <rex/ops/power/sqrt_operation.hpp>
#include <rex/ops/power/square_operation.hpp>

using namespace rex;
using namespace rex::ops;
using xmipp4::test::comparison_mode;
using xmipp4::test::element_value;
using xmipp4::test::elementwise_verb_fixture;

// A result is required exactly only where IEEE-754 requires the operation
// to be correctly rounded, which covers the square, the reciprocal and the
// square root. The library transcendentals carry no such guarantee, so
// those cases are compared within a tolerance for their element type.
//
// The verbs are named with their namespace because <cmath> puts functions
// of the same name in the global one, and naming a verb rather than
// calling it cannot be resolved by its arguments.

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"square multiplies each element by itself",
	"[array_power][cpu]"
)
{
	// (2 + i)^2 == 3 + 4i, so the complex case stays exact as well.
	check_unary<square_operation>(
		xmipp4::square,
		element_value(2, 1),
		[](auto x) { return x * x; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"reciprocal divides one by each element",
	"[array_power][cpu]"
)
{
	check_unary<reciprocal_operation>(
		xmipp4::reciprocal,
		element_value(4),
		[](auto) { return 0.25; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"sqrt takes the square root of each element",
	"[array_power][cpu]"
)
{
	check_unary<sqrt_operation>(
		xmipp4::sqrt,
		element_value(4),
		[](auto) { return 2; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"cbrt takes the cube root of each element",
	"[array_power][cpu]"
)
{
	check_unary<cbrt_operation>(
		xmipp4::cbrt,
		element_value(8),
		[](auto) { return 2; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"exp raises e to each element",
	"[array_power][cpu]"
)
{
	check_unary<exp_operation>(
		xmipp4::exp,
		element_value(1),
		[](auto) { return 2.718281828459045; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"exp2 raises two to each element",
	"[array_power][cpu]"
)
{
	check_unary<exp2_operation>(
		xmipp4::exp2,
		element_value(3),
		[](auto) { return 8; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"expm1 raises e to each element and subtracts one",
	"[array_power][cpu]"
)
{
	check_unary<expm1_operation>(
		xmipp4::expm1,
		element_value(1),
		[](auto) { return 1.718281828459045; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"log takes the natural logarithm of each element",
	"[array_power][cpu]"
)
{
	check_unary<log_operation>(
		xmipp4::log,
		element_value(2),
		[](auto) { return 0.6931471805599453; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"log2 takes the base two logarithm of each element",
	"[array_power][cpu]"
)
{
	check_unary<log2_operation>(
		xmipp4::log2,
		element_value(8),
		[](auto) { return 3; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"log10 takes the base ten logarithm of each element",
	"[array_power][cpu]"
)
{
	check_unary<log10_operation>(
		xmipp4::log10,
		element_value(100),
		[](auto) { return 2; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"log1p takes the natural logarithm of one plus each element",
	"[array_power][cpu]"
)
{
	check_unary<log1p_operation>(
		xmipp4::log1p,
		element_value(1),
		[](auto) { return 0.6931471805599453; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"power raises each element to the corresponding exponent",
	"[array_power][cpu]"
)
{
	// Integers are compared exactly whatever the mode, which is what makes
	// one case enough for a domain holding both them and the complex types
	// that reach this through a logarithm.
	check_binary<power_operation>(
		xmipp4::power,
		element_value(2),
		element_value(3),
		[](auto, auto) { return 8; },
		comparison_mode::approximate
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"power of an integer to a negative exponent is zero",
	"[array_power][cpu]"
)
{
	// The result lies strictly between minus one and one, so truncating it
	// into the operand type leaves nothing. NumPy raises instead, which a
	// per-element kernel cannot do.
	check_binary<power_operation, signed_integer_type_domain>(
		xmipp4::power,
		element_value(2),
		element_value(-1),
		[](auto, auto) { return 0; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"power of one to a negative exponent is still one",
	"[array_power][cpu]"
)
{
	// One and minus one are the two bases whose negative powers remain
	// representable.
	check_binary<power_operation, signed_integer_type_domain>(
		xmipp4::power,
		element_value(1),
		element_value(-3),
		[](auto, auto) { return 1; }
	);
}
