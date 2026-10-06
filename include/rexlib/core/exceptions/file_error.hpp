// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <stdexcept>

#include <rexlib/core/platform/dynamic_shared_object.h>

namespace rexlib
{

/**
 * @brief Exception indicating that a file can not be reached.
 *
 * Thrown when a file is missing or unreadable, or when it can not be
 * created, sized or mapped.
 */
REXLIB_STD_BASE_INTERFACE
class REXLIB_API file_error : public std::runtime_error
{
	using runtime_error::runtime_error;
};

} // namespace rexlib
