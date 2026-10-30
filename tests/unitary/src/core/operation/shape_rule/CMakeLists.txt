# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_any_extent_domain.cpp
		test_core_dimensions.cpp
		test_core_dimensions_descriptor.cpp
		test_core_dimensions_descriptor_table.cpp
		test_extent_domain.cpp
		test_extent_pivot_descriptor.cpp
		test_extent_pivot_descriptor_table.cpp
		test_extent_slot_descriptor.cpp
		test_extent_slot_descriptor_table.cpp
		test_extent_transform.cpp
		test_fixed_extent.cpp
		test_operand_shape_rule.cpp
		test_positive_extent_domain.cpp
		test_same_extent_transform.cpp
		test_shape_rule_accessors.cpp
		test_shape_rule_descriptor.cpp
		test_shape_rule_descriptor_of.cpp
		test_shape_rule_engine.cpp
		test_shape_rule_interpreter.cpp
		test_shape_rule_resolution.cpp
)
