// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/ops/parameters/restored_signal_axes.hpp>

using namespace rexlib;
using namespace rexlib::ops;

TEST_CASE(
	"restored_signal_axes should store its axes and the restored extent",
	"[restored_signal_axes]"
)
{
	const restored_signal_axes parameters(axis_list{ 1, 0 }, 17);
	const auto axes = parameters.get_axes();
	CHECK( axis_list(axes.begin(), axes.end()) == axis_list{ 1, 0 } );
	CHECK( parameters.get_extent() == 17 );
}
