// SPDX-License-Identifier: GPL-3.0-only

#pragma once

/**
 * @file attributes.hpp
 * @brief Macro definitions for supported attributes
 * 
 * This file declares a variety of C++ attributes through
 * macros. When an attribute is not supported, first it
 * tries to provide an alternative though compiler intrinsic
 * macros. If no alternative is available, a dummy declaration
 * is provided. This allows the programmer to use the attributes 
 * through macro definitions with no compatibility check, and
 * then its declaration will handle the best option available.
 */

#include "cpp_attributes.hpp"
#include "c_attributes.h"
#include "builtin.h"

/**
 * @def REXLIB_NORETURN
 * @brief Indicates that the function does not return
 * 
 */
#if REXLIB_HAS_CPP_ATTRIBUTE(noreturn)
	#define REXLIB_NORETURN REXLIB_CPP_ATTRIBUTE(noreturn)
#elif REXLIB_HAS_GCC_ATTRIBUTE(noreturn)
	#define REXLIB_NORETURN REXLIB_GCC_ATTRIBUTE(noreturn)
#else
	#define REXLIB_NORETURN
#endif

/**
 * @def REXLIB_FALLTHROUGH
 * @brief Used to explicitly avoid warnings when a break condition
 * is not desired in a switch statement
 * 
 */
#if REXLIB_HAS_CPP_ATTRIBUTE(fallthrough)
	#define REXLIB_FALLTHROUGH REXLIB_CPP_ATTRIBUTE(fallthrough)
#elif REXLIB_HAS_GCC_ATTRIBUTE(fallthrough)
	#define REXLIB_FALLTHROUGH REXLIB_GCC_ATTRIBUTE(fallthrough)
#else
	#define REXLIB_FALLTHROUGH
#endif

/**
 * @def REXLIB_DEPRECATED(reason)
 * @brief Indicates that a function is deprecated providing a reason
 * 
 */
#if REXLIB_HAS_CPP_ATTRIBUTE(deprecated)
	#if REXLIB_HAS_CPP14
		#define REXLIB_DEPRECATED(reason) REXLIB_CPP_ATTRIBUTE(deprecated(reason))
	#else
		#define REXLIB_DEPRECATED(reason) REXLIB_CPP_ATTRIBUTE(deprecated)
	#endif
#elif REXLIB_HAS_GCC_ATTRIBUTE(deprecated)
	#define REXLIB_DEPRECATED REXLIB_GCC_ATTRIBUTE(deprecated)
#else
	#define REXLIB_DEPRECATED(reason)
#endif

/**
 * @def REXLIB_NODISCARD
 * @brief Indicates that the return value should be used
 * 
 */
#if REXLIB_HAS_CPP_ATTRIBUTE(nodiscard)
	#define REXLIB_NODISCARD REXLIB_CPP_ATTRIBUTE(nodiscard)
#elif REXLIB_HAS_GCC_ATTRIBUTE(warn_unused_result)
	#define REXLIB_NODISCARD REXLIB_GCC_ATTRIBUTE(warn_unused_result)
#else
	#define REXLIB_NODISCARD
#endif

/**
 * @def REXLIB_UNUSED
 * @brief Indicates that a variable is intentionally declared and but unused.
 * 
 */
#if REXLIB_HAS_CPP_ATTRIBUTE(maybe_unused)
	#define REXLIB_UNUSED REXLIB_CPP_ATTRIBUTE(maybe_unused)
#elif REXLIB_HAS_GCC_ATTRIBUTE(unused)
	#define REXLIB_UNUSED REXLIB_GCC_ATTRIBUTE(unused)
#else
	#define REXLIB_UNUSED
#endif

/**
 * @def REXLIB_NODISCARD_MESSAGE(reason)
 * @brief Indicates that the return value should be used, proving a message
 * 
 */
#if REXLIB_HAS_CPP_ATTRIBUTE(nodiscard)
	#if REXLIB_HAS_CPP20
		#define REXLIB_NODISCARD_MESSAGE(reason) REXLIB_CPP_ATTRIBUTE(nodiscard(reason))
	#else
		#define REXLIB_NODISCARD_MESSAGE(reason) REXLIB_CPP_ATTRIBUTE(nodiscard)
	#endif
#elif REXLIB_HAS_GCC_ATTRIBUTE(warn_unused_result)
	#define REXLIB_NODISCARD_MESSAGE(reason) REXLIB_GCC_ATTRIBUTE(warn_unused_result)
#else
	#define REXLIB_NODISCARD_MESSAGE(reason)
#endif

/**
 * @def REXLIB_NO_UNIQUE_ADDRESS
 * @brief Indicates that the empty class does not need to have
 * a unique address and instead it can share space with other
 * variables
 * 
 */
#if REXLIB_HAS_CPP_ATTRIBUTE(no_unique_address)
	#define REXLIB_NO_UNIQUE_ADDRESS REXLIB_CPP_ATTRIBUTE(no_unique_address)
#else
	#define REXLIB_NO_UNIQUE_ADDRESS
#endif

/**
 * @def REXLIB_ASSUME(expr)
 * @brief Assume that a particular expression evaluates to true
 * for compiler optimizations
 * 
 */
#if REXLIB_HAS_CPP_ATTRIBUTE(assume)
	#define REXLIB_ASSUME(expr) REXLIB_CPP_ATTRIBUTE(assume(expr))
#elif REXLIB_HAS_GCC_ATTRIBUTE(assume)
	#define REXLIB_ASSUME(expr) REXLIB_GCC_ATTRIBUTE(assume(expr))
#elif REXLIB_HAS_BUILTIN(__builtin_assume)
	#define REXLIB_ASSUME(expr) __builtin_assume(expr)
#elif defined(__assume)
	#define REXLIB_ASSUME(expr) __assume(expr)
#else
	#define REXLIB_ASSUME(expr)
#endif

/**
 * @def REXLIB_LIKELY
 * @brief Indicates to the compiler that a particular branch is likely
 * 
 */
#if REXLIB_HAS_CPP_ATTRIBUTE(likely)
	#define REXLIB_LIKELY REXLIB_CPP_ATTRIBUTE(likely)
#else
	#define REXLIB_LIKELY
#endif

/**
 * @def REXLIB_UNLIKELY
 * @brief Indicates to the compiler that a particular branch is unlikely
 * 
 */
#if REXLIB_HAS_CPP_ATTRIBUTE(unlikely)
	#define REXLIB_UNLIKELY REXLIB_CPP_ATTRIBUTE(unlikely)
#else
	#define REXLIB_UNLIKELY
#endif
