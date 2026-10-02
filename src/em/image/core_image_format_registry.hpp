// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_read_format_registry.hpp>
#include <rexlib/em/image/image_write_format_registry.hpp>

namespace rexlib
{
namespace em
{

/**
 * @brief Get the registry of the image read formats bundled with the library.
 *
 * Private to the library, so only the formats bundled with it can register
 * into it.
 *
 * @return image_read_format_registry& The registry.
 */
image_read_format_registry& get_core_image_read_format_registry() noexcept;

/**
 * @brief Get the registry of the image write formats bundled with the
 * library.
 *
 * @return image_write_format_registry& The registry.
 *
 * @see get_core_image_read_format_registry
 */
image_write_format_registry& get_core_image_write_format_registry() noexcept;

} // namespace em
} // namespace rexlib
