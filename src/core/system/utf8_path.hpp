// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <boost/filesystem/path.hpp>

#include <string>

namespace rexlib
{

/**
 * @brief Make a path from its spelling in UTF-8.
 *
 * Unlike the constructor of the path, this does not depend on the code
 * page of Windows.
 *
 * @param utf8 The path in UTF-8.
 * @return boost::filesystem::path The path.
 * @throws std::invalid_argument If utf8 is not valid UTF-8 on Windows.
 */
boost::filesystem::path utf8_to_path(const std::string& utf8);

/**
 * @brief Spell a path in UTF-8.
 *
 * Unlike the string of the path, this does not depend on the code page of
 * Windows.
 *
 * @param path The path.
 * @return std::string The path in UTF-8.
 */
std::string path_to_utf8(const boost::filesystem::path& path);

} // namespace rexlib
