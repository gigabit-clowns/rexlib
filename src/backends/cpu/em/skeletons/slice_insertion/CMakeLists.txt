# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		frequency_bounds.cpp
		frequency_interval.cpp
		row_interval.cpp
		slice_entry.cpp
		slice_frame.cpp
		slice_insertion_layout_plan.cpp
		slice_insertion_tuning.cpp
		volume_tile.cpp
		volume_tiling.cpp
)
