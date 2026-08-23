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
 * @def REX_NORETURN
 * @brief Indicates that the function does not return
 * 
 */
#if REX_HAS_CPP_ATTRIBUTE(noreturn)
	#define REX_NORETURN REX_CPP_ATTRIBUTE(noreturn)
#elif REX_HAS_GCC_ATTRIBUTE(noreturn)
	#define REX_NORETURN REX_GCC_ATTRIBUTE(noreturn)
#else
	#define REX_NORETURN
#endif

/**
 * @def REX_FALLTHROUGH
 * @brief Used to explicitly avoid warnings when a break condition
 * is not desired in a switch statement
 * 
 */
#if REX_HAS_CPP_ATTRIBUTE(fallthrough)
	#define REX_FALLTHROUGH REX_CPP_ATTRIBUTE(fallthrough)
#elif REX_HAS_GCC_ATTRIBUTE(fallthrough)
	#define REX_FALLTHROUGH REX_GCC_ATTRIBUTE(fallthrough)
#else
	#define REX_FALLTHROUGH
#endif

/**
 * @def REX_DEPRECATED(reason)
 * @brief Indicates that a function is deprecated providing a reason
 * 
 */
#if REX_HAS_CPP_ATTRIBUTE(deprecated)
	#if REX_HAS_CPP14
		#define REX_DEPRECATED(reason) REX_CPP_ATTRIBUTE(deprecated(reason))
	#else
		#define REX_DEPRECATED(reason) REX_CPP_ATTRIBUTE(deprecated)
	#endif
#elif REX_HAS_GCC_ATTRIBUTE(deprecated)
	#define REX_DEPRECATED REX_GCC_ATTRIBUTE(deprecated)
#else
	#define REX_DEPRECATED(reason)
#endif

/**
 * @def REX_NODISCARD
 * @brief Indicates that the return value should be used
 * 
 */
#if REX_HAS_CPP_ATTRIBUTE(nodiscard)
	#define REX_NODISCARD REX_CPP_ATTRIBUTE(nodiscard)
#elif REX_HAS_GCC_ATTRIBUTE(warn_unused_result)
	#define REX_NODISCARD REX_GCC_ATTRIBUTE(warn_unused_result)
#else
	#define REX_NODISCARD
#endif

/**
 * @def REX_UNUSED
 * @brief Indicates that a variable is intentionally declared and but unused.
 * 
 */
#if REX_HAS_CPP_ATTRIBUTE(maybe_unused)
	#define REX_UNUSED REX_CPP_ATTRIBUTE(maybe_unused)
#elif REX_HAS_GCC_ATTRIBUTE(unused)
	#define REX_UNUSED REX_GCC_ATTRIBUTE(unused)
#else
	#define REX_UNUSED
#endif

/**
 * @def REX_NODISCARD_MESSAGE(reason)
 * @brief Indicates that the return value should be used, proving a message
 * 
 */
#if REX_HAS_CPP_ATTRIBUTE(nodiscard)
	#if REX_HAS_CPP20
		#define REX_NODISCARD_MESSAGE(reason) REX_CPP_ATTRIBUTE(nodiscard(reason))
	#else
		#define REX_NODISCARD_MESSAGE(reason) REX_CPP_ATTRIBUTE(nodiscard)
	#endif
#elif REX_HAS_GCC_ATTRIBUTE(warn_unused_result)
	#define REX_NODISCARD_MESSAGE(reason) REX_GCC_ATTRIBUTE(warn_unused_result)
#else
	#define REX_NODISCARD_MESSAGE(reason)
#endif

/**
 * @def REX_NO_UNIQUE_ADDRESS
 * @brief Indicates that the empty class does not need to have
 * a unique address and instead it can share space with other
 * variables
 * 
 */
#if REX_HAS_CPP_ATTRIBUTE(no_unique_address)
	#define REX_NO_UNIQUE_ADDRESS REX_CPP_ATTRIBUTE(no_unique_address)
#else
	#define REX_NO_UNIQUE_ADDRESS
#endif

/**
 * @def REX_ASSUME(expr)
 * @brief Assume that a particular expression evaluates to true
 * for compiler optimizations
 * 
 */
#if REX_HAS_CPP_ATTRIBUTE(assume)
	#define REX_ASSUME(expr) REX_CPP_ATTRIBUTE(assume(expr))
#elif REX_HAS_GCC_ATTRIBUTE(assume)
	#define REX_ASSUME(expr) REX_GCC_ATTRIBUTE(assume(expr))
#elif REX_HAS_BUILTIN(__builtin_assume)
	#define REX_ASSUME(expr) __builtin_assume(expr)
#elif defined(__assume)
	#define REX_ASSUME(expr) __assume(expr)
#else
	#define REX_ASSUME(expr)
#endif

/**
 * @def REX_LIKELY
 * @brief Indicates to the compiler that a particular branch is likely
 * 
 */
#if REX_HAS_CPP_ATTRIBUTE(likely)
	#define REX_LIKELY REX_CPP_ATTRIBUTE(likely)
#else
	#define REX_LIKELY
#endif

/**
 * @def REX_UNLIKELY
 * @brief Indicates to the compiler that a particular branch is unlikely
 * 
 */
#if REX_HAS_CPP_ATTRIBUTE(unlikely)
	#define REX_UNLIKELY REX_CPP_ATTRIBUTE(unlikely)
#else
	#define REX_UNLIKELY
#endif
