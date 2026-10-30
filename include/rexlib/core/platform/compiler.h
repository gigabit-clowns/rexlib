// SPDX-License-Identifier: GPL-3.0-only

#pragma once

/**
 * @file
 * @brief Macros that name the compiler.
 *
 * At most one of these is defined, to 1:
 * - `REXLIB_CLANG`: Clang.
 * - `REXLIB_ICC`: the Intel C++ Compiler.
 * - `REXLIB_ICX`: the Intel compiler based on LLVM.
 * - `REXLIB_GCC`: the GNU C/C++ compiler.
 * - `REXLIB_MSVC`: Microsoft Visual C++.
 *
 * `REXLIB_MINGW` is defined, to 1, when building with MinGW. It may be
 * defined together with one of the above.
 */

#if defined(__clang__)
	#define REXLIB_CLANG 1
#elif defined(__INTEL_COMPILER)
	#define REXLIB_ICC 1
#elif defined(__INTEL_LLVM_COMPILER)
	#define REXLIB_ICX 1
#elif defined(__GNUC__) || defined(__GNUG__)
	#define REXLIB_GCC 1
#elif defined(_MSC_VER)
	#define REXLIB_MSVC 1
#else
	#pragma message ("Could not determine the compiler")
#endif

#if defined(__MINGW32__) || defined(__MINGW64__)
	#define REXLIB_MINGW 1
#endif
