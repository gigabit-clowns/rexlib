// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/ops/parameters/reduction_axes.hpp>

#include <vector>

using namespace rexlib;
using namespace rexlib::ops;

TEST_CASE(
	"reduction_axes should keep its axes in the order given and drop them "
	"by default",
	"[reduction_axes]"
)
{
	const reduction_axes parameters(axis_list{ 2, 0 });
	const auto axes = parameters.get_axes();
	CHECK( axis_list(axes.begin(), axes.end()) == axis_list{ 2, 0 } );
	CHECK_FALSE( parameters.get_keep_dimensions() );
}

TEST_CASE(
	"reduction_axes should store whether the reduced axes are kept",
	"[reduction_axes]"
)
{
	const reduction_axes parameters(axis_list{ 1 }, true);
	CHECK( parameters.get_keep_dimensions() );
}
