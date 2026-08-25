// SPDX-License-Identifier: GPL-3.0-only

#pragma once

/**
 * @def REXLIB_CLANG
 * @brief Defined when building with Clang
 * 
 */

/**
 * @def REXLIB_ICC
 * @brief Defined when building with Intel Compiler
 * 
 */

/**
 * @def REXLIB_ICX
 * @brief Defined when building with Intel LLVM based Compiler
 * 
 */

/**
 * @def REXLIB_GCC
 * @brief Defined when building with GNU C/C++ compiler
 * 
 */

/**
 * @def REXLIB_MSVC
 * @brief Defined when building with Microsoft Visual C++
 * 
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

/**
 * @def REXLIB_MINGW
 * @brief Defined when building with MinGW.
 * 
 * Unlike the former compiler macros, this definition is not exclusive and
 * it may be present together with other definitions.
 * 
 */
#if defined(__MINGW32__) || defined(__MINGW64__)
	#define REXLIB_MINGW 1
#endif
