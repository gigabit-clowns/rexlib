# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		abs_program_builder.cpp
		add_program_builder.cpp
		divide_program_builder.cpp
		divmod_program_builder.cpp
		floor_divide_program_builder.cpp
		modulo_program_builder.cpp
		multiply_program_builder.cpp
		negate_program_builder.cpp
		sign_program_builder.cpp
		subtract_program_builder.cpp
)
