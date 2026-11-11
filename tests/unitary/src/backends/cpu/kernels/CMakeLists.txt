# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_axis_phase.cpp
		test_bessel_i0.cpp
		test_box_window.cpp
		test_contrast_transfer_function.cpp
		test_eigen_gemm.cpp
		test_frequency_bounds.cpp
		test_kaiser_bessel_window.cpp
		test_ordering.cpp
		test_philox.cpp
		test_pocketfft_fft.cpp
		test_random_key.cpp
		test_row_interval.cpp
		test_slice_frame.cpp
		test_standard_normal.cpp
		test_triangle_window.cpp
)
