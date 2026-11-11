# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_core_input_indices.cpp
		test_default_kernel_factory.cpp
		test_rule_type_dispatch.cpp
		test_typed_program_builder.cpp
)

add_subdirectory(batch_reduce_map)
add_subdirectory(batched_core)
add_subdirectory(elementwise)
add_subdirectory(fourier)
add_subdirectory(reduction)
add_subdirectory(roll)
