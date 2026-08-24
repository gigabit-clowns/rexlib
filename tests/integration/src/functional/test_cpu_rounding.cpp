// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/rounding.hpp>

#include "fixtures/elementwise_verb_fixture.hpp"

#include <rex/ops/rounding/ceil_operation.hpp>
#include <rex/ops/rounding/floor_operation.hpp>
#include <rex/ops/rounding/round_operation.hpp>
#include <rex/ops/rounding/trunc_operation.hpp>

using namespace rexlib;
using namespace rexlib::ops;
using rexlib::test::element_value;
using rexlib::test::elementwise_verb_fixture;

// Every value here is exactly representable in every floating point type,
// float16_t included, and every result is an integer, so these compare
// exactly.

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"ceil takes each element up to an integer",
	"[array_rounding][cpu]"
)
{
	check_unary<ceil_operation>(
		rexlib::ceil,
		element_value(2.25),
		[](auto) { return 3; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"ceil of a negative element goes towards zero",
	"[array_rounding][cpu]"
)
{
	check_unary<ceil_operation>(
		rexlib::ceil,
		element_value(-2.25),
		[](auto) { return -2; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"floor takes each element down to an integer",
	"[array_rounding][cpu]"
)
{
	check_unary<floor_operation>(
		rexlib::floor,
		element_value(2.75),
		[](auto) { return 2; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"floor of a negative element goes away from zero",
	"[array_rounding][cpu]"
)
{
	check_unary<floor_operation>(
		rexlib::floor,
		element_value(-2.25),
		[](auto) { return -3; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"trunc drops the fractional part of each element",
	"[array_rounding][cpu]"
)
{
	check_unary<trunc_operation>(
		rexlib::trunc,
		element_value(2.75),
		[](auto) { return 2; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"trunc of a negative element goes towards zero, unlike floor",
	"[array_rounding][cpu]"
)
{
	check_unary<trunc_operation>(
		rexlib::trunc,
		element_value(-2.75),
		[](auto) { return -2; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"round takes each element to the nearest integer",
	"[array_rounding][cpu]"
)
{
	check_unary<round_operation>(
		rexlib::round,
		element_value(2.75),
		[](auto) { return 3; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"round sends a half to the even neighbour, not away from zero",
	"[array_rounding][cpu]"
)
{
	// std::round would give 1 here. Rounding halves to even is what NumPy
	// does, and what keeps a distribution of them from drifting upwards.
	check_unary<round_operation>(
		rexlib::round,
		element_value(0.5),
		[](auto) { return 0; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"round sends a half up when the lower neighbour is odd",
	"[array_rounding][cpu]"
)
{
	check_unary<round_operation>(
		rexlib::round,
		element_value(1.5),
		[](auto) { return 2; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"round sends a half down when the lower neighbour is even",
	"[array_rounding][cpu]"
)
{
	check_unary<round_operation>(
		rexlib::round,
		element_value(2.5),
		[](auto) { return 2; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"round breaks a negative half towards the even neighbour as well",
	"[array_rounding][cpu]"
)
{
	// std::round would give -3.
	check_unary<round_operation>(
		rexlib::round,
		element_value(-2.5),
		[](auto) { return -2; }
	);
}
