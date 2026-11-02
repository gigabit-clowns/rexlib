# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_alternating_sign.cpp
		test_row_insertion_kernel.cpp
		test_slice_entry.cpp
		test_slice_insertion_program_builder.cpp
		test_tabulated_phase.cpp
		test_unit_weight.cpp
		test_virtual_row.cpp
)
