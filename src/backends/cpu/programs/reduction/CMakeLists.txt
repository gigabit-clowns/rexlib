# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		all_program.cpp
		amax_program.cpp
		amin_program.cpp
		any_program.cpp
		argmax_program.cpp
		argmin_program.cpp
		count_nonzero_program.cpp
		mean_program.cpp
		product_program.cpp
		sum_program.cpp
)
