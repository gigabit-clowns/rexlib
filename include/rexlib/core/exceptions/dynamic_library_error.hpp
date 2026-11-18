// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <stdexcept>

#include <rexlib/core/platform/dynamic_shared_object.h>

namespace rexlib
{

/**
 * @brief Exception indicating that a dynamic library could not be loaded.
 *
 * Its message names the file and gives the reason the system reports, such
 * as a missing file or a symbol that no loaded library defines.
 */
REXLIB_STD_BASE_INTERFACE
class REXLIB_API dynamic_library_error : public std::runtime_error
{
	using runtime_error::runtime_error;
};

} // namespace rexlib
