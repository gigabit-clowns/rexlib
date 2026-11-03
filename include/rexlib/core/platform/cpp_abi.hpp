// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>
#include <cstdint>

/**
 * @file
 * @brief Macros that name the C++ ABI the code is compiled for.
 *
 * - `REXLIB_CPP_ABI_FAMILY`: `REXLIB_CPP_ABI_ITANIUM` or
 *   `REXLIB_CPP_ABI_MSVC`, whichever the compiler follows. GCC and Clang
 *   follow the Itanium ABI, MinGW included; MSVC and clang-cl follow
 *   that of MSVC.
 * - `REXLIB_CPP_STANDARD_LIBRARY`: `REXLIB_CPP_LIBSTDCXX`,
 *   `REXLIB_CPP_LIBCXX`, `REXLIB_CPP_MSVC_STL`, or
 *   `REXLIB_CPP_UNKNOWN_STANDARD_LIBRARY`.
 * - `REXLIB_CPP_STANDARD_LIBRARY_ABI`: the setting that changes the ABI
 *   of that standard library: `_GLIBCXX_USE_CXX11_ABI` for libstdc++,
 *   `_LIBCPP_ABI_VERSION` for libc++, `_ITERATOR_DEBUG_LEVEL` for the
 *   standard library of MSVC, and 0 for any other.
 * - `REXLIB_CPP_DEBUG_RUNTIME`: 1 when the code is built against the
 *   debug runtime of MSVC, and 0 otherwise.
 */

#define REXLIB_CPP_ABI_ITANIUM 1
#define REXLIB_CPP_ABI_MSVC 2

#define REXLIB_CPP_UNKNOWN_STANDARD_LIBRARY 0
#define REXLIB_CPP_LIBSTDCXX 1
#define REXLIB_CPP_LIBCXX 2
#define REXLIB_CPP_MSVC_STL 3

#if defined(_MSC_VER)
	#define REXLIB_CPP_ABI_FAMILY REXLIB_CPP_ABI_MSVC
#else
	#define REXLIB_CPP_ABI_FAMILY REXLIB_CPP_ABI_ITANIUM
#endif

#if defined(_LIBCPP_VERSION)
	#define REXLIB_CPP_STANDARD_LIBRARY REXLIB_CPP_LIBCXX
	#define REXLIB_CPP_STANDARD_LIBRARY_ABI _LIBCPP_ABI_VERSION
#elif defined(__GLIBCXX__)
	#define REXLIB_CPP_STANDARD_LIBRARY REXLIB_CPP_LIBSTDCXX
	#define REXLIB_CPP_STANDARD_LIBRARY_ABI _GLIBCXX_USE_CXX11_ABI
#elif defined(_MSVC_STL_VERSION) || defined(_CPPLIB_VER)
	#if !defined(_ITERATOR_DEBUG_LEVEL)
		#error "The standard library of MSVC left _ITERATOR_DEBUG_LEVEL out"
	#endif
	#define REXLIB_CPP_STANDARD_LIBRARY REXLIB_CPP_MSVC_STL
	#define REXLIB_CPP_STANDARD_LIBRARY_ABI _ITERATOR_DEBUG_LEVEL
#else
	#define REXLIB_CPP_STANDARD_LIBRARY REXLIB_CPP_UNKNOWN_STANDARD_LIBRARY
	#define REXLIB_CPP_STANDARD_LIBRARY_ABI 0
#endif

#if defined(_MSC_VER) && defined(_DEBUG)
	#define REXLIB_CPP_DEBUG_RUNTIME 1
#else
	#define REXLIB_CPP_DEBUG_RUNTIME 0
#endif
