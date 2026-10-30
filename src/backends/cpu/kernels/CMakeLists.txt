# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cpu-obj
	PRIVATE
		eigen_gemm.cpp
		eigen_gemm_complex_float32.cpp
		eigen_gemm_complex_float64.cpp
		eigen_gemm_float32.cpp
		eigen_gemm_float64.cpp
		eigen_gemm_int16.cpp
		eigen_gemm_int32.cpp
		eigen_gemm_int64.cpp
		eigen_gemm_int8.cpp
		eigen_gemm_uint16.cpp
		eigen_gemm_uint32.cpp
		eigen_gemm_uint64.cpp
		eigen_gemm_uint8.cpp
		pocketfft_fourier_transform.cpp
)
