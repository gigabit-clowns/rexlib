// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/bitwise.hpp>

#include <rex/ops/bitwise/bitwise_and_operation.hpp>
#include <rex/ops/bitwise/bitwise_not_operation.hpp>
#include <rex/ops/bitwise/bitwise_or_operation.hpp>
#include <rex/ops/bitwise/bitwise_xor_operation.hpp>
#include <rex/ops/bitwise/left_shift_operation.hpp>
#include <rex/ops/bitwise/right_shift_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

using namespace rex;
using namespace rex::ops;
using rex::test::verb_dispatch_fixture;

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"bitwise_and dispatches a bitwise_and_operation with both inputs in order",
	"[array_bitwise]"
)
{
	check_binary_verb<bitwise_and_operation>(bitwise_and);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"bitwise_or dispatches a bitwise_or_operation with both inputs in order",
	"[array_bitwise]"
)
{
	check_binary_verb<bitwise_or_operation>(bitwise_or);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"bitwise_xor dispatches a bitwise_xor_operation with both inputs in order",
	"[array_bitwise]"
)
{
	check_binary_verb<bitwise_xor_operation>(bitwise_xor);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"bitwise_not dispatches a bitwise_not_operation with its single input",
	"[array_bitwise]"
)
{
	check_unary_verb<bitwise_not_operation>(bitwise_not);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"left_shift dispatches a left_shift_operation with both inputs in order",
	"[array_bitwise]"
)
{
	check_binary_verb<left_shift_operation>(left_shift);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"right_shift dispatches a right_shift_operation with both inputs in order",
	"[array_bitwise]"
)
{
	check_binary_verb<right_shift_operation>(right_shift);
}
