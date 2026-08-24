// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/functional/numeric.hpp>

#include <rexlib/ops/numeric/copysign_operation.hpp>
#include <rexlib/ops/numeric/is_finite_operation.hpp>
#include <rexlib/ops/numeric/is_inf_operation.hpp>
#include <rexlib/ops/numeric/is_nan_operation.hpp>
#include <rexlib/ops/numeric/sign_bit_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

using namespace rexlib;
using namespace rexlib::ops;
using rexlib::test::verb_dispatch_fixture;

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"is_nan dispatches a is_nan_operation with its single input",
	"[array_numeric]"
)
{
	check_unary_verb<is_nan_operation>(is_nan);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"is_inf dispatches a is_inf_operation with its single input",
	"[array_numeric]"
)
{
	check_unary_verb<is_inf_operation>(is_inf);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"is_finite dispatches a is_finite_operation with its single input",
	"[array_numeric]"
)
{
	check_unary_verb<is_finite_operation>(is_finite);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"sign_bit dispatches a sign_bit_operation with its single input",
	"[array_numeric]"
)
{
	check_unary_verb<sign_bit_operation>(sign_bit);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"copysign dispatches a copysign_operation with both inputs in order",
	"[array_numeric]"
)
{
	check_binary_verb<copysign_operation>(copysign);
}
