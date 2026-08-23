// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "stringfy.h"

/**
 * @def REX_PRAGMA
 * @brief Provide a directive to the compiler.
 * 
 */
#define REX_PRAGMA(x) _Pragma(REX_STRINGFY(x))
