// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/span.hpp>

namespace rexlib
{
namespace em
{
namespace tiff
{

/**
 * @brief Check whether a file begins as a TIFF file does.
 *
 * A TIFF file begins with the two bytes that state its byte order followed
 * by its version in that order, which is 42 for a classic file and 43 for a
 * BigTIFF one.
 *
 * @param leading_bytes The front of the file.
 * @return bool true if they are the signature of either form in either byte
 * order.
 */
bool has_signature(span<const byte> leading_bytes) noexcept;

} // namespace tiff
} // namespace em
} // namespace rexlib
