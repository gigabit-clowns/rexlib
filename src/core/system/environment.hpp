// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>

namespace rexlib
{

/**
 * @brief Get the value of an environment variable in UTF-8.
 *
 * Unlike std::getenv, this does not depend on the code page of Windows.
 *
 * @param name The name of the variable, in ASCII.
 * @return std::string Its value, empty when it is not set.
 */
std::string get_environment_variable(const std::string& name);

} // namespace rexlib
