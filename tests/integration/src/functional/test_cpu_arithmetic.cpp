// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/arithmetic.hpp>

#include "fixtures/elementwise_verb_fixture.hpp"

#include <rex/ops/arithmetic/abs_operation.hpp>
#include <rex/ops/arithmetic/add_operation.hpp>
#include <rex/ops/arithmetic/divide_operation.hpp>
#include <rex/ops/arithmetic/divmod_operation.hpp>
#include <rex/ops/arithmetic/floor_divide_operation.hpp>
#include <rex/ops/arithmetic/modulo_operation.hpp>
#include <rex/ops/arithmetic/multiply_operation.hpp>
#include <rex/ops/arithmetic/negate_operation.hpp>
#include <rex/ops/arithmetic/sign_operation.hpp>
#include <rex/ops/arithmetic/subtract_operation.hpp>

#include <rex/functional/creation.hpp>
#include <rex/core/hardware/memory_resource_affinity.hpp>
#include <rex/core/ndarray/const_array_ref.hpp>
#include <rex/core/numerical/scalar_value.hpp>

#include <cmath>
#include <complex>

using namespace rexlib;
using namespace rexlib::ops;
using rexlib::test::comparison_mode;
using rexlib::test::element_value;
using rexlib::test::elementwise_verb_fixture;

// Each case below sweeps every numerical type, running the verb for the
// types the operation's rule admits and requiring a rejection for the rest.
// Neither set is written down here: both come from the operation itself.
//
// A reference is spelled as the C++ operator when the operation is that
// operator, and as a constant when the operation deliberately departs from
// it, so that a divergence like the sign of a modulo is asserted against a
// value rather than against a second copy of the kernel.

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"add computes the element-wise sum of two arrays",
	"[array_arithmetic][cpu]"
)
{
	// Booleans reach the kernel's disjunction and characters its integral
	// promotion, both of which agree with the sum written here.
	check_binary<add_operation>(
		add,
		element_value(2, 1),
		element_value(3, 2),
		[](auto x, auto y) { return x + y; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"subtract computes the element-wise difference of two arrays",
	"[array_arithmetic][cpu]"
)
{
	check_binary<subtract_operation>(
		subtract,
		element_value(5, 2),
		element_value(2, 1),
		[](auto x, auto y) { return x - y; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"multiply computes the element-wise product of two arrays",
	"[array_arithmetic][cpu]"
)
{
	check_binary<multiply_operation>(
		multiply,
		element_value(2, 1),
		element_value(3, 2),
		[](auto x, auto y) { return x * y; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"divide computes the element-wise quotient of two arrays",
	"[array_arithmetic][cpu]"
)
{
	// Operands chosen so the quotient is exact for every admitted type,
	// integer division included. Dividing by a real-valued complex number
	// keeps the complex case just as simple.
	check_binary<divide_operation>(
		divide,
		element_value(8, 4),
		element_value(2, 0),
		[](auto x, auto y) { return x / y; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"divide gives a fractional result for floating point arrays, "
	"unlike integer division",
	"[array_arithmetic][cpu]"
)
{
	check_binary<divide_operation, floating_point_type_domain>(
		divide,
		element_value(9),
		element_value(2),
		[](auto, auto) { return 4.5; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"modulo computes the element-wise modulo of two arrays",
	"[array_arithmetic][cpu]"
)
{
	check_binary<modulo_operation>(
		modulo,
		element_value(7),
		element_value(3),
		[](auto, auto) { return 1; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"modulo result takes the sign of the divisor for a negative dividend, "
	"unlike C's modulo",
	"[array_arithmetic][cpu]"
)
{
	// Restricted to the types that can hold a negative dividend at all.
	// Complex types are in that domain but not in modulo's, so this still
	// requires them to be rejected.
	check_binary<modulo_operation, signed_arithmetic_type_domain>(
		modulo,
		element_value(-7),
		element_value(3),
		[](auto, auto) { return 2; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"modulo result takes the sign of the divisor for a negative divisor, "
	"unlike C's modulo",
	"[array_arithmetic][cpu]"
)
{
	check_binary<modulo_operation, signed_arithmetic_type_domain>(
		modulo,
		element_value(7),
		element_value(-3),
		[](auto, auto) { return -2; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"floor_divide computes the element-wise quotient rounded down",
	"[array_arithmetic][cpu]"
)
{
	check_binary<floor_divide_operation>(
		floor_divide,
		element_value(7),
		element_value(3),
		[](auto, auto) { return 2; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"floor_divide rounds towards negative infinity, unlike C's division",
	"[array_arithmetic][cpu]"
)
{
	// C truncates -7 / 3 to -2. Rounding down gives -3, which is what
	// keeps floor_divide and modulo the two halves of one division.
	check_binary<floor_divide_operation, signed_arithmetic_type_domain>(
		floor_divide,
		element_value(-7),
		element_value(3),
		[](auto, auto) { return -3; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"divmod computes the quotient and the remainder of one division",
	"[array_arithmetic][cpu]"
)
{
	// The pair satisfies quotient * divisor + remainder == dividend, which
	// C's truncating division would break for a negative dividend.
	check_binary_pair<divmod_operation, signed_arithmetic_type_domain>(
		divmod,
		element_value(-7),
		element_value(3),
		[](auto, auto) { return -3; },
		[](auto, auto) { return 2; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"sign reports the sign of each element",
	"[array_arithmetic][cpu]"
)
{
	// A negative real, and a complex number lying along the negative real
	// axis, both report the same unit magnitude.
	check_unary<sign_operation, signed_arithmetic_type_domain>(
		sign,
		element_value(-3),
		[](auto) { return -1; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"sign is zero for a zero element",
	"[array_arithmetic][cpu]"
)
{
	check_unary<sign_operation>(
		sign,
		element_value(0),
		[](auto) { return 0; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"sign of a complex element is the unit vector along it",
	"[array_arithmetic][cpu]"
)
{
	// A Pythagorean pair again, so dividing by the magnitude is exact.
	check_unary<sign_operation, complex_type_domain>(
		sign,
		element_value(3, 4),
		[](auto x) { return x / std::abs(x); }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"negate computes the element-wise negation of an array",
	"[array_arithmetic][cpu]"
)
{
	check_unary<negate_operation>(
		negate,
		element_value(2, 1),
		[](auto x) { return -x; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"abs computes the element-wise absolute value of a signed array, "
	"producing the real equivalent type for a complex one",
	"[array_arithmetic][cpu]"
)
{
	// The parts are a Pythagorean pair, so the magnitude of the complex
	// case is exact in every precision.
	check_unary<abs_operation, signed_arithmetic_type_domain>(
		rexlib::abs,
		element_value(-3, -4),
		[](auto x) { using std::abs; return abs(x); }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"abs is the identity for an unsigned array",
	"[array_arithmetic][cpu]"
)
{
	check_unary<abs_operation, unsigned_integer_type_domain>(
		rexlib::abs,
		element_value(3),
		[](auto x) { return x; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"an elementwise verb reuses the storage of the provided output array",
	"[array_arithmetic][cpu]"
)
{
	// Output reuse belongs to the builder rather than to any one kernel,
	// so it is asserted once here instead of for every verb.
	const auto descriptor = make_descriptor({ 2, 3 });

	auto x = full(
		descriptor, memory_resource_affinity::device,
		scalar_value(2.0f), context
	);
	auto y = full(
		descriptor, memory_resource_affinity::device,
		scalar_value(3.0f), context
	);
	const_array_ref x_ref = x;
	const_array_ref y_ref = y;

	auto out = zeros(descriptor, memory_resource_affinity::device, context);
	const auto *out_storage = out.get_storage();

	const auto result = add(x_ref, y_ref, context, &out);

	CHECK( result.get_storage() == out_storage );
	for (const auto value : read_host<float>(result, 6))
	{
		CHECK( value == 5.0f );
	}
}
