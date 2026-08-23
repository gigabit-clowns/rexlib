// SPDX-License-Identifier: GPL-3.0-only

#pragma once

/**
 * @def REX_CLANG
 * @brief Defined when building with Clang
 * 
 */

/**
 * @def REX_ICC
 * @brief Defined when building with Intel Compiler
 * 
 */

/**
 * @def REX_ICX
 * @brief Defined when building with Intel LLVM based Compiler
 * 
 */

/**
 * @def REX_GCC
 * @brief Defined when building with GNU C/C++ compiler
 * 
 */

/**
 * @def REX_MSVC
 * @brief Defined when building with Microsoft Visual C++
 * 
 */

#if defined(__clang__)
	#define REX_CLANG 1
#elif defined(__INTEL_COMPILER)
	#define REX_ICC 1
#elif defined(__INTEL_LLVM_COMPILER)
	#define REX_ICX 1
#elif defined(__GNUC__) || defined(__GNUG__)
	#define REX_GCC 1
#elif defined(_MSC_VER)
	#define REX_MSVC 1
#else
	#pragma message ("Could not determine the compiler")
#endif

/**
 * @def REX_MINGW
 * @brief Defined when building with MinGW.
 * 
 * Unlike the former compiler macros, this definition is not exclusive and
 * it may be present together with other definitions.
 * 
 */
#if defined(__MINGW32__) || defined(__MINGW64__)
	#define REX_MINGW 1
#endif
