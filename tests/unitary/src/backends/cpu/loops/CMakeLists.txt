# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_claimed_loop.cpp
		test_elementwise_loop.cpp
		test_parallel_grain.cpp
		test_reduction_element_adaptor.cpp
		test_reduction_loop.cpp
)

add_subdirectory(indices)
add_subdirectory(layouts)
add_subdirectory(strides)
