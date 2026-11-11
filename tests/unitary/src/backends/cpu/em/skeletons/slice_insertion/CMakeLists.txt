# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_insertion_chunk.cpp
		test_row_insertion_kernel.cpp
		test_slice_entry.cpp
		test_slice_insertion_functor.cpp
		test_slice_insertion_layout_plan.cpp
		test_slice_insertion_program_builder.cpp
		test_slice_insertion_tuning.cpp
		test_virtual_row.cpp
		test_volume_tile.cpp
		test_volume_tiling.cpp
)
