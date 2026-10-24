// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/shape_rule/shape_rule_descriptor.hpp>

#include <rexlib/core/operation/shape_rule/any_extent_domain.hpp>
#include <rexlib/core/span.hpp>

#include <array>

using namespace rexlib;

TEST_CASE(
	"shape_rule_descriptor should store its tables, placement and output "
	"batch",
	"[shape_rule_descriptor]"
)
{
	const std::array<extent_pivot_descriptor, 1> extents = {{
		extent_pivot_descriptor(
			extent_pivot_source::inputs,
			make_extent_domain<any_extent_domain>()
		)
	}};
	const std::array<core_dimensions_descriptor, 1> outputs = {{
		core_dimensions_descriptor(span<const axis_slot_descriptor>())
	}};
	const std::array<core_dimensions_descriptor, 2> inputs = {{
		core_dimensions_descriptor(span<const axis_slot_descriptor>()),
		core_dimensions_descriptor(span<const axis_slot_descriptor>())
	}};

	const shape_rule_descriptor descriptor(
		make_span(extents),
		make_span(outputs),
		make_span(inputs),
		core_placement::named,
		output_batch::broadcast_target
	);

	CHECK( descriptor.get_extents().data() == extents.data() );
	CHECK( descriptor.get_extents().size() == 1 );
	CHECK( descriptor.get_output_cores().data() == outputs.data() );
	CHECK( descriptor.get_output_cores().size() == 1 );
	CHECK( descriptor.get_input_cores().data() == inputs.data() );
	CHECK( descriptor.get_input_cores().size() == 2 );
	CHECK( descriptor.get_placement() == core_placement::named );
	CHECK( descriptor.get_output_batch() == output_batch::broadcast_target );
}
