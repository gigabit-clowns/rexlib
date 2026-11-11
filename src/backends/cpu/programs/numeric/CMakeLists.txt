# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		copysign_program.cpp
		is_finite_program.cpp
		is_inf_program.cpp
		is_nan_program.cpp
		sign_bit_program.cpp
)
