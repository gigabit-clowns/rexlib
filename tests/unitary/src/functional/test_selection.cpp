// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <xmipp4/functional/selection.hpp>

#include <xmipp4/ops/selection/clip_operation.hpp>
#include <xmipp4/ops/selection/maximum_operation.hpp>
#include <xmipp4/ops/selection/minimum_operation.hpp>
#include <xmipp4/ops/selection/where_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

using namespace rex;
using namespace rex::ops;
using xmipp4::test::verb_dispatch_fixture;

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"where dispatches a where_operation with all three inputs in order",
	"[array_selection]"
)
{
	// The condition is boolean while the operands selected between are not,
	// which is the point of the rule this operation uses: its pivot comes
	// from the second input rather than the first.
	check_ternary_verb<where_operation>(
		where,
		numerical_type::boolean,
		numerical_type::float32
	);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"clip dispatches a clip_operation with all three inputs in order",
	"[array_selection]"
)
{
	check_ternary_verb<clip_operation>(clip);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"maximum dispatches a maximum_operation with both inputs in order",
	"[array_selection]"
)
{
	check_binary_verb<maximum_operation>(maximum);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"minimum dispatches a minimum_operation with both inputs in order",
	"[array_selection]"
)
{
	check_binary_verb<minimum_operation>(minimum);
}
