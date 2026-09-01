// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <stdexcept>

#include <rexlib/core/platform/dynamic_shared_object.h>

namespace rexlib
{
namespace em
{

/**
 * @brief Exception indicating that an access is outside what a format offers.
 *
 * Thrown when the request itself is well formed but the format can not serve
 * it, such as opening a file for writing in a format that only reads.
 * A request that is malformed for any format reports @c std::invalid_argument
 * instead.
 */
REXLIB_STD_BASE_INTERFACE
class REXLIB_API unsupported_image_access_error : public std::logic_error
{
	using logic_error::logic_error;
};

} // namespace em
} // namespace rexlib
