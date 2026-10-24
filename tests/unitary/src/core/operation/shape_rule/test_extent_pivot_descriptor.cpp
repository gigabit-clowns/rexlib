// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/shape_rule/extent_pivot_descriptor.hpp>

#include <rexlib/core/operation/shape_rule/positive_extent_domain.hpp>

using namespace rexlib;

TEST_CASE(
	"extent_pivot_descriptor should store its source and domain",
	"[extent_pivot_descriptor]"
)
{
	const extent_pivot_descriptor descriptor(
		extent_pivot_source::operation,
		make_extent_domain<positive_extent_domain>()
	);
	CHECK( descriptor.get_source() == extent_pivot_source::operation );
	CHECK_FALSE( descriptor.get_domain().contains(0) );
	CHECK( descriptor.get_domain().contains(2) );
}
