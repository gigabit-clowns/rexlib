# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		batch_reduce_map_plan.cpp
		fourier_layout_plan.cpp
		index_operands.cpp
		linalg_core_layout_plan.cpp
		linalg_operand_core.cpp
		named_core_frame.cpp
		reduction_layout_plan.cpp
		roll_layout_plan.cpp
)
