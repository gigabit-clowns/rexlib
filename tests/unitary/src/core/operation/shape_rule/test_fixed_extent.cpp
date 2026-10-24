// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/shape_rule/fixed_extent.hpp>

using namespace rexlib;

namespace
{

struct test_parameters
{
};

} // anonymous namespace

TEST_CASE(
	"fixed_extent should yield its extent whatever the parameters",
	"[fixed_extent]"
)
{
	CHECK( fixed_extent<3>::get(test_parameters()) == 3 );
	CHECK( fixed_extent<0>::get(0) == 0 );
}
