// SPDX-License-Identifier: GPL-3.0-only

#pragma once

/**
 * @brief Declaration name of a builtin function
 * 
 */
#define REXLIB_BUILTIN(fun) __builtin_##fun

/**
 * @def REXLIB_HAS_BUILTIN(fun)
 * @brief Checks if a particular builtin function is provided
 * 
 * When no way of checking availability is provided, it 
 * defaults to false.
 * 
 */
#if defined(__has_builtin) && !defined(REXLIB_NO_BUILTIN)
	#define REXLIB_HAS_BUILTIN(fun) __has_builtin(__builtin_##fun)
#elif defined(REXLIB_DOC_BUILD)
	#define REXLIB_HAS_BUILTIN(fun) 1
#else
	#define REXLIB_HAS_BUILTIN(fun) 0
#endif
