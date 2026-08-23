// SPDX-License-Identifier: GPL-3.0-only

#pragma once

/**
 * @def REX_WINDOWS
 * @brief Defined if the target is Windows (32bit or 64bit)
 * 
 */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
	#define REX_WINDOWS 1
#endif

/**
 * @def REX_APPLE
 * @brief Defined if the target is MacOS
 * 
 */
#if defined(__APPLE__) || defined(__MACH__)
	#define REX_APPLE 1
#endif

/**
 * @def REX_UNIX
 * @brief Defined if the target is Unix-like, including Linux and MacOS
 * 
 */
#if defined(__unix__) || defined(__unix) || defined(REX_APPLE)
	#define REX_UNIX 1
#endif

/**
 * @def REX_LINUX
 * @brief Defined if the target is Linux
 * 
 */
#if defined(__linux__) || defined(__linux) || defined(__gnu_linux__)
	#define REX_LINUX 1
#endif

/**
 * @def REX_BSD
 * @brief Defined if the target is BSD
 * 
 */
#if defined(BSD)
	#define REX_BSD 1
#endif

/**
 * @def REX_POSIX
 * @brief Defined if the target is POSIX
 * 
 */
#if defined(_POSIX_VERSION) || defined(REX_UNIX)
	#define REX_POSIX 1
#endif
