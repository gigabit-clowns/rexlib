// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_write_format_registry.hpp>

#include <rexlib/em/image/image_write_format.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

namespace rexlib
{
namespace em
{

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

} // namespace em
} // namespace rexlib
