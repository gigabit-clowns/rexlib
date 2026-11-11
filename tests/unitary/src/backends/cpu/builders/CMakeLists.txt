# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_batch_reduce_map_program_builder.cpp
		test_batched_core_program_builder.cpp
		test_complex_fourier_transform.cpp
		test_complex_to_real_fourier_transform.cpp
		test_core_input_indices.cpp
		test_default_kernel_factory.cpp
		test_elementwise_program_builder.cpp
		test_extremum_locator_kernel.cpp
		test_fold_reduction_kernel.cpp
		test_fourier_conversion.cpp
		test_fourier_program_builder.cpp
		test_real_to_complex_fourier_transform.cpp
		test_rule_type_dispatch.cpp
		test_typed_program_builder.cpp
)

add_subdirectory(em)
