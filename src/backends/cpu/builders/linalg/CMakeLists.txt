# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		cross_program_builder.cpp
		matmul_program_builder.cpp
		matvec_program_builder.cpp
		vecdot_program_builder.cpp
		vecmat_program_builder.cpp
)
