// SPDX-License-Identifier: GPL-3.0-only

#pragma once

/**
 * @def REXLIB_WINDOWS
 * @brief Defined if the target is Windows (32bit or 64bit)
 * 
 */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
	#define REXLIB_WINDOWS 1
#endif

/**
 * @def REXLIB_APPLE
 * @brief Defined if the target is MacOS
 * 
 */
#if defined(__APPLE__) || defined(__MACH__)
	#define REXLIB_APPLE 1
#endif

/**
 * @def REXLIB_UNIX
 * @brief Defined if the target is Unix-like, including Linux and MacOS
 * 
 */
#if defined(__unix__) || defined(__unix) || defined(REXLIB_APPLE)
	#define REXLIB_UNIX 1
#endif

/**
 * @def REXLIB_LINUX
 * @brief Defined if the target is Linux
 * 
 */
#if defined(__linux__) || defined(__linux) || defined(__gnu_linux__)
	#define REXLIB_LINUX 1
#endif

/**
 * @def REXLIB_BSD
 * @brief Defined if the target is BSD
 * 
 */
#if defined(BSD)
	#define REXLIB_BSD 1
#endif

/**
 * @def REXLIB_POSIX
 * @brief Defined if the target is POSIX
 * 
 */
#if defined(_POSIX_VERSION) || defined(REXLIB_UNIX)
	#define REXLIB_POSIX 1
#endif
