// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>

namespace rexlib
{

/**
 * @brief Convert UTF-8 to the UTF-16 of the Windows API.
 *
 * @param utf8 The text in UTF-8.
 * @return std::wstring The text in UTF-16.
 * @throws std::invalid_argument If utf8 is not valid UTF-8.
 */
std::wstring utf8_to_utf16(const std::string& utf8);

/**
 * @brief Convert the UTF-16 of the Windows API to UTF-8.
 *
 * A lone surrogate, which a Windows file name may hold, becomes U+FFFD.
 *
 * @param utf16 The text in UTF-16.
 * @return std::string The text in UTF-8.
 */
std::string utf16_to_utf8(const std::wstring& utf16);

} // namespace rexlib
