// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/shape_rule/core_dimensions_descriptor.hpp>

#include <rexlib/core/operation/shape_rule/any_extent_domain.hpp>
#include <rexlib/core/operation/shape_rule/positive_extent_domain.hpp>
#include <rexlib/core/operation/shape_rule/same_extent_transform.hpp>
#include <rexlib/core/span.hpp>

#include <array>
#include <string>

using namespace rexlib;

namespace
{

extent_slot_descriptor make_slot(std::size_t pivot)
{
	return extent_slot_descriptor(
		pivot,
		make_extent_transform<same_extent_transform>()
	);
}

} // anonymous namespace

TEST_CASE(
	"core_dimensions_descriptor should hold its slots",
	"[core_dimensions_descriptor]"
)
{
	const std::array<extent_slot_descriptor, 2> slots = {{
		make_slot(0),
		make_slot(1)
	}};

	const core_dimensions_descriptor descriptor(
		make_span(slots.data(), slots.size()),
		false,
		make_extent_domain<any_extent_domain>()
	);
	CHECK( descriptor.get_slots().size() == 2 );
	CHECK( descriptor.get_slots().data() == slots.data() );
	CHECK_FALSE( descriptor.has_run() );
}

TEST_CASE(
	"core_dimensions_descriptor should hold the domain of its run",
	"[core_dimensions_descriptor]"
)
{
	const core_dimensions_descriptor descriptor(
		span<const extent_slot_descriptor>(),
		true,
		make_extent_domain<positive_extent_domain>()
	);
	CHECK( descriptor.get_slots().empty() );
	CHECK( descriptor.has_run() );
	CHECK_FALSE( descriptor.get_run_domain().contains(0) );
	CHECK( descriptor.get_run_domain().contains(1) );
	CHECK(
		std::string(descriptor.get_run_domain().get_description()) ==
		positive_extent_domain::get_description()
	);
}

TEST_CASE(
	"core_dimensions_descriptor should be empty only without a run and "
	"without a slot",
	"[core_dimensions_descriptor]"
)
{
	const std::array<extent_slot_descriptor, 1> slots = {{ make_slot(0) }};
	const auto domain = make_extent_domain<any_extent_domain>();

	CHECK(
		core_dimensions_descriptor(
			span<const extent_slot_descriptor>(),
			false,
			domain
		).is_empty()
	);
	CHECK_FALSE(
		core_dimensions_descriptor(
			span<const extent_slot_descriptor>(),
			true,
			domain
		).is_empty()
	);
	CHECK_FALSE(
		core_dimensions_descriptor(
			make_span(slots.data(), slots.size()),
			false,
			domain
		).is_empty()
	);
}
