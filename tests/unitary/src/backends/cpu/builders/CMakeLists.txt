target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_default_kernel_factory.cpp
		test_dispatched_program_builder.cpp
		test_elementwise_program_builder.cpp
		test_extremum_locator_kernel.cpp
		test_fold_reduction_kernel.cpp
		test_fourier_program_builder.cpp
		test_linalg_program_builder.cpp
)

add_subdirectory(type_dispatchers)
