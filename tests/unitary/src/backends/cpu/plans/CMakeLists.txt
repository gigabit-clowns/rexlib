# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_batch_reduce_map_plan.cpp
		test_fourier_layout_plan.cpp
		test_frequency_interval.cpp
		test_index_operands.cpp
		test_linalg_core_layout_plan.cpp
		test_operand_core.cpp
		test_named_core_frame.cpp
		test_roll_layout_plan.cpp
		test_slice_insertion_layout_plan.cpp
		test_volume_tile.cpp
		test_volume_tiling.cpp
)
