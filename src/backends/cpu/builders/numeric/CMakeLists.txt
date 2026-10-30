# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		copysign_program_builder.cpp
		is_finite_program_builder.cpp
		is_inf_program_builder.cpp
		is_nan_program_builder.cpp
		sign_bit_program_builder.cpp
)
