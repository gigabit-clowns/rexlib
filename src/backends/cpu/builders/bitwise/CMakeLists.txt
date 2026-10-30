# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		bitwise_and_program_builder.cpp
		bitwise_not_program_builder.cpp
		bitwise_or_program_builder.cpp
		bitwise_xor_program_builder.cpp
		left_shift_program_builder.cpp
		right_shift_program_builder.cpp
)
