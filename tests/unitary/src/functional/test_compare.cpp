// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <xmipp4/functional/compare.hpp>

#include <xmipp4/ops/compare/equal_operation.hpp>
#include <xmipp4/ops/compare/greater_operation.hpp>
#include <xmipp4/ops/compare/greater_equal_operation.hpp>
#include <xmipp4/ops/compare/less_operation.hpp>
#include <xmipp4/ops/compare/less_equal_operation.hpp>
#include <xmipp4/ops/compare/not_equal_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

using namespace rex;
using namespace rex::ops;
using xmipp4::test::verb_dispatch_fixture;

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"equal dispatches a equal_operation with both inputs in order",
	"[array_compare]"
)
{
	check_binary_verb<equal_operation>(equal);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"not_equal dispatches a not_equal_operation with both inputs in order",
	"[array_compare]"
)
{
	check_binary_verb<not_equal_operation>(not_equal);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"less dispatches a less_operation with both inputs in order",
	"[array_compare]"
)
{
	check_binary_verb<less_operation>(less);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"less_equal dispatches a less_equal_operation with both inputs in order",
	"[array_compare]"
)
{
	check_binary_verb<less_equal_operation>(less_equal);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"greater dispatches a greater_operation with both inputs in order",
	"[array_compare]"
)
{
	check_binary_verb<greater_operation>(greater);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"greater_equal dispatches a greater_equal_operation with both inputs in order",
	"[array_compare]"
)
{
	check_binary_verb<greater_equal_operation>(greater_equal);
}
