// SPDX-License-Identifier: GPL-3.0-only

#pragma once

/**
 * @def REXLIB_ENUM_TO_STR_CASE(ns, x)
 * @brief Generate a switch-case arm that returns the stringified enum value name.
 *
 * Expands to: `case ns::x: return "x"`;
 *
 * @param ns The enum class name (used as the scope qualifier).
 * @param x  The enumerator name.
 */
#define REXLIB_ENUM_TO_STR_CASE(ns, x) case ns::x: return #x;
