// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/ops/parameters/axis_selection.hpp>

using namespace rexlib;
using namespace rexlib::ops;

TEST_CASE(
	"axis_selection should keep its axes in the order given",
	"[axis_selection]"
)
{
	const axis_selection parameters(axis_list{ 1, 3, 0 });
	const auto axes = parameters.get_axes();
	CHECK( axis_list(axes.begin(), axes.end()) == axis_list{ 1, 3, 0 } );
}

TEST_CASE(
	"axis_selection may name no axis",
	"[axis_selection]"
)
{
	const axis_selection parameters{ axis_list() };
	CHECK( parameters.get_axes().empty() );
}
