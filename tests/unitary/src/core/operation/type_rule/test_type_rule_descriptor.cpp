// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/type_rule_descriptor.hpp>

#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>
#include <rexlib/core/operation/type_rule/same_type_transform.hpp>

#include <array>

using namespace rexlib;

TEST_CASE(
	"type_rule_descriptor should hold the tables it is given",
	"[type_rule_descriptor]"
)
{
	const std::array<type_pivot_descriptor, 1> pivots = {{
		type_pivot_descriptor(
			type_pivot_source::input,
			0,
			-1,
			any_type_domain::get()
		)
	}};
	const auto same = make_type_transform<same_type_transform>();
	const std::array<type_slot_descriptor, 1> outputs = {{
		type_slot_descriptor(0, same)
	}};
	const std::array<type_slot_descriptor, 2> inputs = {{
		type_slot_descriptor(0, same),
		type_slot_descriptor(0, same)
	}};

	const type_rule_descriptor rule(
		make_span(pivots.data(), pivots.size()),
		make_span(outputs.data(), outputs.size()),
		make_span(inputs.data(), inputs.size())
	);

	CHECK( rule.get_pivots().data() == pivots.data() );
	CHECK( rule.get_pivots().size() == 1 );
	CHECK( rule.get_output_slots().data() == outputs.data() );
	CHECK( rule.get_output_slots().size() == 1 );
	CHECK( rule.get_input_slots().data() == inputs.data() );
	CHECK( rule.get_input_slots().size() == 2 );
}
