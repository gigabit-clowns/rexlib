// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/rules/core_dimensions_descriptor.hpp>

#include <rexlib/core/dispatch/rules/any_extent_domain.hpp>
#include <rexlib/core/dispatch/rules/same_extent_transform.hpp>
#include <rexlib/core/span.hpp>

#include <array>

using namespace rexlib;

namespace
{

axis_slot_descriptor make_slot(axis_slot_kind kind)
{
	return axis_slot_descriptor(
		kind,
		0,
		&same_extent_transform::apply,
		same_extent_transform::get_relation(),
		make_extent_domain<any_extent_domain>()
	);
}

} // anonymous namespace

TEST_CASE(
	"core_dimensions_descriptor without a run should count every slot as "
	"an axis",
	"[core_dimensions_descriptor]"
)
{
	const std::array<axis_slot_descriptor, 2> slots = {{
		make_slot(axis_slot_kind::extent),
		make_slot(axis_slot_kind::fixed)
	}};

	const core_dimensions_descriptor descriptor(make_span(slots));
	CHECK( descriptor.get_slots().size() == 2 );
	CHECK( descriptor.get_slots().data() == slots.data() );
	CHECK_FALSE( descriptor.has_run() );
	CHECK( descriptor.get_run_position() == 2 );
	CHECK( descriptor.get_axis_count() == 2 );
}

TEST_CASE(
	"core_dimensions_descriptor should find its run",
	"[core_dimensions_descriptor]"
)
{
	const std::array<axis_slot_descriptor, 3> slots = {{
		make_slot(axis_slot_kind::extent),
		make_slot(axis_slot_kind::run),
		make_slot(axis_slot_kind::extent)
	}};

	const core_dimensions_descriptor descriptor(make_span(slots));
	CHECK( descriptor.has_run() );
	CHECK( descriptor.get_run_position() == 1 );
	CHECK( descriptor.get_axis_count() == 2 );
}

TEST_CASE(
	"core_dimensions_descriptor without slots should declare no core axis",
	"[core_dimensions_descriptor]"
)
{
	const core_dimensions_descriptor descriptor{
		span<const axis_slot_descriptor>()
	};
	CHECK_FALSE( descriptor.has_run() );
	CHECK( descriptor.get_run_position() == 0 );
	CHECK( descriptor.get_axis_count() == 0 );
}
