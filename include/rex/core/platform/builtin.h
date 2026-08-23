// SPDX-License-Identifier: GPL-3.0-only

#pragma once

/**
 * @brief Declaration name of a builtin function
 * 
 */
#define REX_BUILTIN(fun) __builtin_##fun

/**
 * @def REX_HAS_BUILTIN(fun)
 * @brief Checks if a particular builtin function is provided
 * 
 * When no way of checking availability is provided, it 
 * defaults to false.
 * 
 */
#if defined(__has_builtin) && !defined(REX_NO_BUILTIN)
	#define REX_HAS_BUILTIN(fun) __has_builtin(__builtin_##fun)
#elif defined(REX_DOC_BUILD)
	#define REX_HAS_BUILTIN(fun) 1
#else
	#define REX_HAS_BUILTIN(fun) 0
#endif
