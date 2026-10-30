# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		cpu_program_builder.cpp
)

add_subdirectory(builders)
add_subdirectory(hardware)
add_subdirectory(kernels)
add_subdirectory(loops)
add_subdirectory(plans)
