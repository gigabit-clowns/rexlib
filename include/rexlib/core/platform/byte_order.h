// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operating_system.h"

/**
 * @brief Constant representing little endian byte ordering
 * 
 */
#define REXLIB_BYTE_ORDER_LITTLE_ENDIAN 1

/**
 * @brief Constant representing big endian byte ordering
 * 
 */
#define REXLIB_BYTE_ORDER_BIG_ENDIAN 2


/**
 * @def REXLIB_BYTE_ORDER
 * @brief Indicates the byte ordering used by the system
 * 
 * Detection based on rapidjson code
 */

//Detect with GCC 4.6's macro.
#if defined(__BYTE_ORDER__)
	#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
		#define REXLIB_BYTE_ORDER REXLIB_BYTE_ORDER_LITTLE_ENDIAN
	#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
		#define REXLIB_BYTE_ORDER REXLIB_BYTE_ORDER_BIG_ENDIAN
	#else
		#error "Unknown byte order"
	#endif

// Detect with _LITTLE_ENDIAN and _BIG_ENDIAN macro.
#elif defined(_LITTLE_ENDIAN)
	#define REXLIB_BYTE_ORDER REXLIB_BYTE_ORDER_LITTLE_ENDIAN
#elif defined(_BIG_ENDIAN)
	#define REXLIB_BYTE_ORDER REXLIB_BYTE_ORDER_BIG_ENDIAN

// Detect for Windows
#elif defined(REXLIB_WINDOWS)
	#include <Windows.h>
	#if REG_DWORD == REG_DWORD_LITTLE_ENDIAN
		#define REXLIB_BYTE_ORDER REXLIB_BYTE_ORDER_LITTLE_ENDIAN
	#elif REG_DWORD == REG_DWORD_BIG_ENDIAN
		#define REXLIB_BYTE_ORDER REXLIB_BYTE_ORDER_BIG_ENDIAN
	#endif

// Failed detecting 
#else
	#error "Could not determine byte ordering"
#endif

/**
 * @def REXLIB_FLOAT_BYTE_ORDER
 * @brief Indicates the byte ordering used by the Floating
 * Point Unit of the system
 * 
 */
#if defined(__FLOAT_WORD_ORDER__)
	#if (__FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__)
		#define REXLIB_FLOAT_BYTE_ORDER REXLIB_BYTE_ORDER_LITTLE_ENDIAN
	#elif (__FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__)
		#define REXLIB_FLOAT_BYTE_ORDER REXLIB_BYTE_ORDER_BIG_ENDIAN
	#else
		#error "Unknown float byte order"
	#endif

#else
	#define REXLIB_FLOAT_BYTE_ORDER REXLIB_BYTE_ORDER
#endif
