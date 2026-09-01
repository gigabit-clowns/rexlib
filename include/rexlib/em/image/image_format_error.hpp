// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <stdexcept>

#include <rexlib/core/platform/dynamic_shared_object.h>

namespace rexlib
{
namespace em
{

/**
 * @brief Exception indicating that an image file can not be interpreted.
 *
 * Thrown when the contents of a file contradict the format that is reading
 * it: a malformed or truncated header, a declared size that the file can not
 * hold, or an encoding that the format recognizes but does not implement.
 */
REXLIB_STD_BASE_INTERFACE
class REXLIB_API image_format_error : public std::runtime_error
{
	using runtime_error::runtime_error;
};

} // namespace em
} // namespace rexlib
