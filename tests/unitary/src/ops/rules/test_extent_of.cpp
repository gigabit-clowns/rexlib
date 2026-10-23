// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/ops/rules/extent_of.hpp>

#include <rexlib/ops/parameters/restored_signal_axes.hpp>

using namespace rexlib::ops;

TEST_CASE(
	"extent_of should read the extent of the parameters",
	"[extent_of]"
)
{
	CHECK( extent_of::get(restored_signal_axes(axis_list{ 0 }, 9)) == 9 );
}
