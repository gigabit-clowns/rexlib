// SPDX-License-Identifier: GPL-3.0-only

#include "core_image_format_registry.hpp"

namespace rexlib
{
namespace em
{

image_read_format_registry& get_core_image_read_format_registry() noexcept
{
	static image_read_format_registry registry;
	return registry;
}

image_write_format_registry& get_core_image_write_format_registry() noexcept
{
	static image_write_format_registry registry;
	return registry;
}

} // namespace em
} // namespace rexlib
