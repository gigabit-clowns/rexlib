# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		cross_program.cpp
		matmul_program.cpp
		matvec_program.cpp
		vecdot_program.cpp
		vecmat_program.cpp
)
