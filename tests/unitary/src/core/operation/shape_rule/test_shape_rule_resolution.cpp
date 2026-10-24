// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <core/operation/shape_rule/shape_rule_resolution.hpp>

using namespace rexlib;

namespace
{

using extent_list_type = shape_rule_resolution::extent_list_type;

} // anonymous namespace

TEST_CASE(
	"shape_rule_resolution should store the batch, the extents and the run",
	"[shape_rule_resolution]"
)
{
	const shape_rule_resolution resolution(
		extent_list_type{ 4, 1 },
		extent_list_type{ 3, 8 },
		extent_list_type{ 5, 6 }
	);

	CHECK( resolution.get_batch() == extent_list_type{ 4, 1 } );
	CHECK( resolution.get_extent(0) == 3 );
	CHECK( resolution.get_extent(1) == 8 );
	CHECK( resolution.get_run_extents() == extent_list_type{ 5, 6 } );
}
