// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_format_registry.hpp>

#include <rexlib/em/image/image_read_format.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>
#include <rexlib/em/image/image_write_format.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

#include <em/image/core_image_format_registry.hpp>

namespace rexlib
{
namespace em
{

image_read_format_registry::image_read_format_registry() = default;
image_read_format_registry::~image_read_format_registry() = default;

void image_read_format_registry::add(image_read_format_factory factory)
{
	if (factory)
	{
		m_factories.push_back(factory);
	}
}

void image_read_format_registry::register_all(
	image_read_format_manager &manager
) const
{
	for (const auto factory : m_factories)
	{
		manager.register_format(factory());
	}
}

image_write_format_registry::image_write_format_registry() = default;
image_write_format_registry::~image_write_format_registry() = default;

void image_write_format_registry::add(image_write_format_factory factory)
{
	if (factory)
	{
		m_factories.push_back(factory);
	}
}

void image_write_format_registry::register_all(
	image_write_format_manager &manager
) const
{
	for (const auto factory : m_factories)
	{
		manager.register_format(factory());
	}
}

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
