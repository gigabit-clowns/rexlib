# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		fft_program_builder.cpp
		fftshift_program_builder.cpp
		ifft_program_builder.cpp
		ifftshift_program_builder.cpp
		irfft_program_builder.cpp
		rfft_program_builder.cpp
)
