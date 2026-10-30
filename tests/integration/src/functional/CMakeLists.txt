# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-integration-tests
	PRIVATE
		test_cpu_arithmetic.cpp
		test_cpu_bitwise.cpp
		test_cpu_cast.cpp
		test_cpu_compare.cpp
		test_cpu_complex.cpp
		test_cpu_creation.cpp
		test_cpu_fourier.cpp
		test_cpu_linalg.cpp
		test_cpu_logical.cpp
		test_cpu_numeric.cpp
		test_cpu_power.cpp
		test_cpu_reduction.cpp
		test_cpu_rounding.cpp
		test_cpu_selection.cpp
		test_cpu_transfer.cpp
		test_cpu_trigonometric.cpp
)
