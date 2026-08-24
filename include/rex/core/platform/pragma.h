// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "stringfy.h"

/**
 * @def REXLIB_PRAGMA
 * @brief Provide a directive to the compiler.
 * 
 */
#define REXLIB_PRAGMA(x) _Pragma(REXLIB_STRINGFY(x))
