# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		batch_reduce_map_plan.cpp
		batched_core_layout_plan.cpp
		fourier_layout_plan.cpp
		frequency_interval.cpp
		reduction_layout_plan.cpp
		roll_layout_plan.cpp
		slice_insertion_layout_plan.cpp
		volume_tile.cpp
		volume_tiling.cpp
)
