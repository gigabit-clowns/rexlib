# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		all_program_builder.cpp
		amax_program_builder.cpp
		amin_program_builder.cpp
		any_program_builder.cpp
		argmax_program_builder.cpp
		argmin_program_builder.cpp
		count_nonzero_program_builder.cpp
		mean_program_builder.cpp
		product_program_builder.cpp
		sum_program_builder.cpp
)
