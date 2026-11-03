// SPDX-License-Identifier: GPL-3.0-only

#include "builtin_plugin_registry.hpp"

#include <rexlib/core/platform/assert.hpp>

#include <utility>

namespace rexlib
{

void builtin_plugin_registry::add(std::unique_ptr<plugin> instance)
{
	REXLIB_ASSERT(instance);
	m_plugins.push_back(std::move(instance));
}

void builtin_plugin_registry::register_all(service_catalog &catalog) const
{
	for (const auto &instance : m_plugins)
	{
		instance->register_at(catalog);
	}
}

builtin_plugin_registry& get_builtin_plugin_registry() noexcept
{
	static builtin_plugin_registry registry;
	return registry;
}

} // namespace rexlib
