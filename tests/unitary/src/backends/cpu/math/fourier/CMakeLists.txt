# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_alternating_sign.cpp
		test_axis_phase.cpp
		test_frequency_bounds.cpp
		test_frequency_interval.cpp
		test_row_interval.cpp
		test_slice_frame.cpp
		test_tabulated_phase.cpp
)
