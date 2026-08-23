// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/bitwise.hpp>

#include "fixtures/elementwise_verb_fixture.hpp"

#include <rex/ops/bitwise/bitwise_and_operation.hpp>
#include <rex/ops/bitwise/bitwise_not_operation.hpp>
#include <rex/ops/bitwise/bitwise_or_operation.hpp>
#include <rex/ops/bitwise/bitwise_xor_operation.hpp>
#include <rex/ops/bitwise/left_shift_operation.hpp>
#include <rex/ops/bitwise/right_shift_operation.hpp>

using namespace rex;
using namespace rex::ops;
using rex::test::element_value;
using rex::test::elementwise_verb_fixture;

// The combining operations admit booleans and characters as well as the
// integers, and a boolean holds only the bits zero and one. Cases over the
// whole domain therefore use operands of zero and one, where every type
// agrees; the multi-bit cases are restricted to the integers, which are
// the types that have more than one bit of value to combine.

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"bitwise_and keeps the bits set in both elements",
	"[array_bitwise][cpu]"
)
{
	check_binary<bitwise_and_operation>(
		rex::bitwise_and,
		element_value(1),
		element_value(0),
		[](auto, auto) { return 0; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"bitwise_and combines the bits of two integers",
	"[array_bitwise][cpu]"
)
{
	// 0b1100 & 0b1010 == 0b1000
	check_binary<bitwise_and_operation, integral_type_domain>(
		rex::bitwise_and,
		element_value(12),
		element_value(10),
		[](auto, auto) { return 8; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"bitwise_or keeps the bits set in either element",
	"[array_bitwise][cpu]"
)
{
	check_binary<bitwise_or_operation>(
		rex::bitwise_or,
		element_value(1),
		element_value(0),
		[](auto, auto) { return 1; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"bitwise_or combines the bits of two integers",
	"[array_bitwise][cpu]"
)
{
	// 0b1100 | 0b1010 == 0b1110
	check_binary<bitwise_or_operation, integral_type_domain>(
		rex::bitwise_or,
		element_value(12),
		element_value(10),
		[](auto, auto) { return 14; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"bitwise_xor keeps the bits set in exactly one element",
	"[array_bitwise][cpu]"
)
{
	check_binary<bitwise_xor_operation>(
		rex::bitwise_xor,
		element_value(1),
		element_value(1),
		[](auto, auto) { return 0; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"bitwise_xor combines the bits of two integers",
	"[array_bitwise][cpu]"
)
{
	// 0b1100 ^ 0b1010 == 0b0110
	check_binary<bitwise_xor_operation, integral_type_domain>(
		rex::bitwise_xor,
		element_value(12),
		element_value(10),
		[](auto, auto) { return 6; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"bitwise_not complements the bits of a signed integer",
	"[array_bitwise][cpu]"
)
{
	// Complementing a two's complement integer negates it and subtracts
	// one. Restricted to the signed types, the result for an unsigned one
	// depending on its width.
	check_unary<bitwise_not_operation, signed_integer_type_domain>(
		rex::bitwise_not,
		element_value(12),
		[](auto) { return -13; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"bitwise_not negates a boolean rather than complementing its storage",
	"[array_bitwise][cpu]"
)
{
	// Complementing the byte a boolean occupies would give -2, which
	// converts back to true and would leave the operation doing nothing.
	check_unary<bitwise_not_operation, boolean_type_domain>(
		rex::bitwise_not,
		element_value(1),
		[](auto) { return false; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"left_shift moves the bits of each element up",
	"[array_bitwise][cpu]"
)
{
	// 0b0011 << 2 == 0b1100
	check_binary<left_shift_operation>(
		rex::left_shift,
		element_value(3),
		element_value(2),
		[](auto, auto) { return 12; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"right_shift moves the bits of each element down",
	"[array_bitwise][cpu]"
)
{
	// 0b1100 >> 2 == 0b0011
	check_binary<right_shift_operation>(
		rex::right_shift,
		element_value(12),
		element_value(2),
		[](auto, auto) { return 3; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"right_shift of a negative element keeps its sign",
	"[array_bitwise][cpu]"
)
{
	// An arithmetic shift, which C++20 requires and every earlier
	// implementation performed anyway.
	check_binary<right_shift_operation, signed_integer_type_domain>(
		rex::right_shift,
		element_value(-12),
		element_value(2),
		[](auto, auto) { return -3; }
	);
}
