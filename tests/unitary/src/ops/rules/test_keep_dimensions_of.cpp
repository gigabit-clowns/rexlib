// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/ops/rules/keep_dimensions_of.hpp>

#include <rexlib/ops/parameters/reduction_axes.hpp>

using namespace rexlib::ops;

TEST_CASE(
	"keep_dimensions_of should read whether the parameters keep the axes",
	"[keep_dimensions_of]"
)
{
	CHECK( keep_dimensions_of::get(reduction_axes(axis_list{ 0 }, true)) );
	CHECK_FALSE( keep_dimensions_of::get(reduction_axes(axis_list{ 0 })) );
}
