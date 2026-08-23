// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <xmipp4/functional/logical.hpp>

#include <xmipp4/ops/logical/logical_and_operation.hpp>
#include <xmipp4/ops/logical/logical_not_operation.hpp>
#include <xmipp4/ops/logical/logical_or_operation.hpp>
#include <xmipp4/ops/logical/logical_xor_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

using namespace rex;
using namespace rex::ops;
using xmipp4::test::verb_dispatch_fixture;

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"logical_and dispatches a logical_and_operation with both inputs in order",
	"[array_logical]"
)
{
	check_binary_verb<logical_and_operation>(logical_and);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"logical_or dispatches a logical_or_operation with both inputs in order",
	"[array_logical]"
)
{
	check_binary_verb<logical_or_operation>(logical_or);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"logical_xor dispatches a logical_xor_operation with both inputs in order",
	"[array_logical]"
)
{
	check_binary_verb<logical_xor_operation>(logical_xor);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"logical_not dispatches a logical_not_operation with its single input",
	"[array_logical]"
)
{
	check_unary_verb<logical_not_operation>(logical_not);
}
