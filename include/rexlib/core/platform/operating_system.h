// SPDX-License-Identifier: GPL-3.0-only

#pragma once

/**
 * @file
 * @brief Macros that name the target operating system.
 *
 * Each of these is defined, to 1, when the target is what it names. More
 * than one may be defined:
 * - `REXLIB_WINDOWS`: Windows, 32 or 64 bit.
 * - `REXLIB_APPLE`: macOS.
 * - `REXLIB_UNIX`: a Unix-like system, Linux and macOS included.
 * - `REXLIB_LINUX`: Linux.
 * - `REXLIB_BSD`: BSD.
 * - `REXLIB_POSIX`: a POSIX system.
 */

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
	#define REXLIB_WINDOWS 1
#endif

#if defined(__APPLE__) || defined(__MACH__)
	#define REXLIB_APPLE 1
#endif

#if defined(__unix__) || defined(__unix) || defined(REXLIB_APPLE)
	#define REXLIB_UNIX 1
#endif

#if defined(__linux__) || defined(__linux) || defined(__gnu_linux__)
	#define REXLIB_LINUX 1
#endif

#if defined(BSD)
	#define REXLIB_BSD 1
#endif

#if defined(_POSIX_VERSION) || defined(REXLIB_UNIX)
	#define REXLIB_POSIX 1
#endif
