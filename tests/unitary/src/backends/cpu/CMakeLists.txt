# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_cpu_plugin.cpp
)

add_subdirectory(dispatch)
add_subdirectory(eigen)
add_subdirectory(em)
add_subdirectory(fourier_shift)
add_subdirectory(hardware)
add_subdirectory(loops)
add_subdirectory(numerical)
add_subdirectory(pocketfft)
add_subdirectory(random)
add_subdirectory(regions)
add_subdirectory(skeletons)
