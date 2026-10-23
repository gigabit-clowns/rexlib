// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/ops/rules/axes_of.hpp>

#include <rexlib/ops/parameters/axis_selection.hpp>

using namespace rexlib;
using namespace rexlib::ops;

TEST_CASE(
	"axes_of should read the axes of the parameters",
	"[axes_of]"
)
{
	const axis_selection parameters(axis_list{ 2, 1 });
	const auto axes = axes_of::get(parameters);
	CHECK( axes.data() == parameters.get_axes().data() );
	CHECK( axes.size() == 2 );
}
