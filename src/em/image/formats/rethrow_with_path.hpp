// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/attributes.hpp>

#include <string>

namespace rexlib
{
namespace em
{

/**
 * @brief Rethrow the exception being handled, its message prefixed with the
 * path of the file it concerns.
 *
 * The exception is rethrown as the same type when it is one of those an
 * image reader or writer reports: @ref image_file_error,
 * @ref image_format_error, @ref unsupported_operation_error,
 * @ref unsupported_capability_error, @c std::out_of_range or
 * @c std::invalid_argument. Any other exception is rethrown unchanged.
 *
 * Only callable while an exception is being handled.
 *
 * @param path Path to the file the exception concerns.
 */
REXLIB_NORETURN
void rethrow_with_path(const std::string &path);

} // namespace em
} // namespace rexlib
