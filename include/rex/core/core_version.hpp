// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "version.hpp"
#include "platform/dynamic_shared_object.h"

namespace rex 
{

/**
 * @brief Get the version of the rexlib installation
 * 
 * This function returns the version of the loaded rexlib .so
 * file
 * 
 * @return version Version of the installation
 */
REXLIB_API
version get_core_version() noexcept;

} // namespace rex
