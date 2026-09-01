// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_format_registry.hpp>

namespace rexlib
{
namespace em
{

/**
 * @brief Get the registry of the image read formats bundled with the library.
 *
 * Declared in a private header so that external modules can not name it, and
 * so can not auto-register into it. A plugin owns and drains its own
 * @ref image_read_format_registry instead.
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
