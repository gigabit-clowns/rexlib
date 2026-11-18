# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_complex_fourier_transform.cpp
		test_complex_to_real_fourier_transform.cpp
		test_fourier_conversion.cpp
		test_fourier_layout_plan.cpp
		test_fourier_program_builder.cpp
		test_real_to_complex_fourier_transform.cpp
)
