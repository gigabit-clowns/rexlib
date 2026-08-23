// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/rounding.hpp>

#include <rex/ops/rounding/ceil_operation.hpp>
#include <rex/ops/rounding/floor_operation.hpp>
#include <rex/ops/rounding/round_operation.hpp>
#include <rex/ops/rounding/trunc_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

using namespace rex;
using namespace rex::ops;
using xmipp4::test::verb_dispatch_fixture;

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"floor dispatches a floor_operation with its single input",
	"[array_rounding]"
)
{
	check_unary_verb<floor_operation>(floor);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"ceil dispatches a ceil_operation with its single input",
	"[array_rounding]"
)
{
	check_unary_verb<ceil_operation>(ceil);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"trunc dispatches a trunc_operation with its single input",
	"[array_rounding]"
)
{
	check_unary_verb<trunc_operation>(trunc);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"round dispatches a round_operation with its single input",
	"[array_rounding]"
)
{
	check_unary_verb<round_operation>(round);
}
