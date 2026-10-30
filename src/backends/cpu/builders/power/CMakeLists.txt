# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		cbrt_program_builder.cpp
		exp2_program_builder.cpp
		exp_program_builder.cpp
		expm1_program_builder.cpp
		log10_program_builder.cpp
		log1p_program_builder.cpp
		log2_program_builder.cpp
		log_program_builder.cpp
		power_program_builder.cpp
		reciprocal_program_builder.cpp
		sqrt_program_builder.cpp
		square_program_builder.cpp
)
