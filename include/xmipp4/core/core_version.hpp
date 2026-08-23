// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "version.hpp"
#include "platform/dynamic_shared_object.h"

namespace xmipp4 
{

/**
 * @brief Get the version of the xmipp-core installation
 * 
 * This function returns the version of the loaded xmipp-core .so
 * file
 * 
 * @return version Version of the installation
 */
REXLIB_API
version get_core_version() noexcept;

} // namespace xmipp4
