// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "platform/dynamic_shared_object.h"

#include <memory>
#include <string>

namespace xmipp4
{

class plugin;
class service_catalog;

/**
 * @brief Class managing a set of plugins.
 * 
 */
class plugin_manager
{
public:
	REXLIB_API plugin_manager();
	plugin_manager(const plugin_manager& other) = delete;
	REXLIB_API plugin_manager(plugin_manager&& other) noexcept;
	REXLIB_API ~plugin_manager();

	plugin_manager& operator=(const plugin_manager& other) = delete;
	REXLIB_API plugin_manager& operator=(plugin_manager&& other) noexcept;

	/**
	 * @brief Add a already loaded plugin.
	 * 
	 * @param plugin The plugin to be added.
	 */
	REXLIB_API void add_plugin(const plugin& plugin);

	/**
	 * @brief Load a plugin from a Shared Object and add it.
	 * 
	 * The path to the shared object can be provided without a extension,
	 * in which case it will be inferred from the platform. Relative paths
	 * will be searched in LD_LIBRARY_PATH.
	 * 
	 * @param path Path to the plugin. 
	 * @return const plugin* The loaded plugin. nullptr if it could not be 
	 * loaded.
	 */
	REXLIB_API const plugin* load_plugin(const std::string &path);

	/**
	 * @brief Get the number of plugins known by this manager.
	 * 
	 * @return std::size_t The number of plugins.
	 */
	REXLIB_API std::size_t get_plugin_count() const noexcept;

	/**
	 * @brief Get the i-th plugin.
	 * 
	 * @param index The index of the requested plugin. Must be less than
	 * the plugin count.
	 * @return const plugin& The requested plugin.
	 */
	REXLIB_API const plugin& get_plugin(std::size_t index) const;

private:
	class implementation;
	std::unique_ptr<implementation> m_implementation;

	void create_if_null();
};


/**
 * @brief Get the default plugin directory.
 * 
 * Returns the path to a sub-directory named as "plugins" located
 * in the same directory as the shared library of the core.
 * 
 * @return std::string Default plugin directory.
 * 
 */
REXLIB_API 
std::string get_default_plugin_directory();

/**
 * @brief Get the the configured plugin directory.
 * 
 * Returns the value of XMIPP4_PLUGINS_DIRECTORY environment variable,
 * if set. Otherwise it returns the default plugin directory
 * 
 * @return std::string The plugin directory.
 * @see get_default_plugin_directory
 * 
 */
REXLIB_API 
std::string get_plugin_directory();

/**
 * @brief Discover and load all plugins in a directory.
 * 
 * @param directory Directory where plugins are searched.
 * @param manager Plugin manager where plugins are loaded.
 * 
 */
REXLIB_API
void discover_plugins(const std::string& directory, plugin_manager &manager);

/**
 * @brief Discover and load all plugins in the default directory.
 * 
 * @param manager Plugin manager where plugins are loaded.
 * 
 */
REXLIB_API
void discover_plugins(plugin_manager &manager);

/**
 * @brief Register all plugins from a plugin manager into an
 * interface catalog.
 * 
 * @param manager The plugin_manager that contains the plugins to
 * be registered.
 * @param catalog Interface catalog where plugins will be registered.
 * @return std::size_t Number of plugins registered.
 */
REXLIB_API
std::size_t register_all_plugins_at(
	const plugin_manager &manager, 
	service_catalog &catalog
	);

} // namespace xmipp4
