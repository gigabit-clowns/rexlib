# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cuda-unitary-tests
	PRIVATE
		test_cuda_plugin.cpp
)
add_subdirectory(hardware)
