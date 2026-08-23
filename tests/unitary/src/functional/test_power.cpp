// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/power.hpp>

#include <rex/ops/power/cbrt_operation.hpp>
#include <rex/ops/power/exp_operation.hpp>
#include <rex/ops/power/exp2_operation.hpp>
#include <rex/ops/power/expm1_operation.hpp>
#include <rex/ops/power/log_operation.hpp>
#include <rex/ops/power/log10_operation.hpp>
#include <rex/ops/power/log1p_operation.hpp>
#include <rex/ops/power/log2_operation.hpp>
#include <rex/ops/power/power_operation.hpp>
#include <rex/ops/power/reciprocal_operation.hpp>
#include <rex/ops/power/sqrt_operation.hpp>
#include <rex/ops/power/square_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

using namespace rex;
using namespace rex::ops;
using xmipp4::test::verb_dispatch_fixture;

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"power dispatches a power_operation with both inputs in order",
	"[array_power]"
)
{
	check_binary_verb<power_operation>(power);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"square dispatches a square_operation with its single input",
	"[array_power]"
)
{
	check_unary_verb<square_operation>(square);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"reciprocal dispatches a reciprocal_operation with its single input",
	"[array_power]"
)
{
	check_unary_verb<reciprocal_operation>(reciprocal);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"sqrt dispatches a sqrt_operation with its single input",
	"[array_power]"
)
{
	check_unary_verb<sqrt_operation>(sqrt);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"cbrt dispatches a cbrt_operation with its single input",
	"[array_power]"
)
{
	check_unary_verb<cbrt_operation>(cbrt);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"exp dispatches a exp_operation with its single input",
	"[array_power]"
)
{
	check_unary_verb<exp_operation>(exp);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"exp2 dispatches a exp2_operation with its single input",
	"[array_power]"
)
{
	check_unary_verb<exp2_operation>(exp2);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"expm1 dispatches a expm1_operation with its single input",
	"[array_power]"
)
{
	check_unary_verb<expm1_operation>(expm1);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"log dispatches a log_operation with its single input",
	"[array_power]"
)
{
	check_unary_verb<log_operation>(log);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"log2 dispatches a log2_operation with its single input",
	"[array_power]"
)
{
	check_unary_verb<log2_operation>(log2);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"log10 dispatches a log10_operation with its single input",
	"[array_power]"
)
{
	check_unary_verb<log10_operation>(log10);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"log1p dispatches a log1p_operation with its single input",
	"[array_power]"
)
{
	check_unary_verb<log1p_operation>(log1p);
}
