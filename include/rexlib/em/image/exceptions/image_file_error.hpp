// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <stdexcept>

#include <rexlib/core/platform/dynamic_shared_object.h>

namespace rexlib
{
namespace em
{

/**
 * @brief Exception indicating that an image file can not be reached.
 *
 * Thrown when the file itself is the obstacle, whatever it holds: it is
 * missing or unreadable, or it can not be created, sized, mapped or flushed.
 * A file that is reached but whose contents contradict its format is an
 * @ref image_format_error instead.
 */
REXLIB_STD_BASE_INTERFACE
class REXLIB_API image_file_error : public std::runtime_error
{
	using runtime_error::runtime_error;
};

} // namespace em
} // namespace rexlib
