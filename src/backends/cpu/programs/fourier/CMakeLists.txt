# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		fft_program.cpp
		fftshift_program.cpp
		ifft_program.cpp
		ifftshift_program.cpp
		irfft_program.cpp
		rfft_program.cpp
)
