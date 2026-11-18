// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/plugin.hpp>

#include <memory>
#include <vector>

namespace rexlib
{

class service_catalog;

/**
 * @brief Hold the plugins built into the library.
 *
 * A catalog that registers built-in plugins registers each of them at
 * itself, as a loaded plugin is registered.
 */
class builtin_plugin_registry
{
public:
	builtin_plugin_registry() = default;
	builtin_plugin_registry(const builtin_plugin_registry &other) = delete;
	builtin_plugin_registry(builtin_plugin_registry &&other) = delete;
	~builtin_plugin_registry() = default;

	builtin_plugin_registry& operator=(
		const builtin_plugin_registry &other
	) = delete;
	builtin_plugin_registry& operator=(
		builtin_plugin_registry &&other
	) = delete;

	/**
	 * @brief Add a plugin.
	 *
	 * @param instance The plugin. Must not be null.
	 */
	void add(std::unique_ptr<plugin> instance);

	/**
	 * @brief Register every plugin added so far at a catalog.
	 *
	 * @param catalog The catalog to register the plugins at.
	 */
	void register_all(service_catalog &catalog) const;

private:
	std::vector<std::unique_ptr<plugin>> m_plugins;
};

/**
 * @brief Access the registry of the plugins built into the library.
 *
 * Built-in plugins add themselves to it during static initialization,
 * through a @ref builtin_plugin_registration.
 *
 * @return builtin_plugin_registry& The registry.
 */
builtin_plugin_registry& get_builtin_plugin_registry() noexcept;

} // namespace rexlib
