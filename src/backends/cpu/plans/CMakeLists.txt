# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		batch_reduce_map_plan.cpp
		fourier_layout_plan.cpp
		frequency_interval.cpp
		index_operands.cpp
		linalg_core_layout_plan.cpp
		operand_core.cpp
		named_core_frame.cpp
		reduction_layout_plan.cpp
		roll_layout_plan.cpp
		slice_insertion_layout_plan.cpp
		volume_tile.cpp
		volume_tiling.cpp
)
