# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		insert_ctf_slices_program_builder.cpp
		insert_slices_program_builder.cpp
		insert_weighted_slices_program_builder.cpp
		slice_entry.cpp
		slice_insertion_tuning.cpp
)
