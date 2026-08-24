// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "c_features.h"
#include "cpp_version.hpp"

#if REXLIB_HAS_CPP20
# include <version>
#endif

/**
 * @brief Macro to test if a C++ feature is available
 * @param feature Name of the feature (without the __cpp_ prefix)
 * @param version Requested standard version as YYYYMM
 * @see https://en.cppreference.com/w/cpp/feature_test
 * 
 */
#define REXLIB_HAS_CPP_FEATURE(feature, version) \
	((__cpp_##feature >= version) || \
	(REXLIB_HAS_C_FEATURE(cxx_##feature) && REXLIB_CPLUSPLUS >= version))

/**
 * @def REXLIB_HAS_CONSTEXPR 
 * @brief Tests if constexpr is supported by the compiler
 * 
 */
#if REXLIB_HAS_CPP_FEATURE(constexpr, 200704L)
#	define REXLIB_HAS_CONSTEXPR 1
#elif REXLIB_HAS_CPP11 && defined(_MSC_VER)
#	if _MSC_VER >= 1900
#		define REXLIB_HAS_CONSTEXPR 1
#	else
#		define REXLIB_HAS_CONSTEXPR 0
#	endif
#else
#	define REXLIB_HAS_CONSTEXPR 0
#endif

#define REXLIB_HAS_IF_CONSTEXPR REXLIB_HAS_CPP_FEATURE(if_constexpr, 201606L)
#define REXLIB_HAS_LIB_BITOPS REXLIB_HAS_CPP_FEATURE(lib_bitops, 201907)
#define REXLIB_HAS_LIB_POW2 REXLIB_HAS_CPP_FEATURE(lib_pow2, 202002)
#define REXLIB_HAS_LIB_EXECUTION REXLIB_HAS_CPP_FEATURE(lib_execution, 201603)
#define REXLIB_HAS_CONDITIONAL_EXPLICIT REXLIB_HAS_CPP_FEATURE(conditional_explicit, 201806L)
