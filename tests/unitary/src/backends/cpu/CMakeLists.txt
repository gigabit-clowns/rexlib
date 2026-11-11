# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_cpu_plugin.cpp
)

add_subdirectory(dispatch)
add_subdirectory(em)
add_subdirectory(hardware)
add_subdirectory(libraries)
add_subdirectory(loops)
add_subdirectory(numerical)
add_subdirectory(random)
add_subdirectory(skeletons)
