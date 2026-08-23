// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/system/dynamic_library.hpp>

namespace rex
{

class plugin;

/**
 * @brief Helper class to load plugins from shared objects. 
 * 
 * RAII style to free resources when destroyed.
 * 
 */
class plugin_loader
{
public:
	plugin_loader();

	/**
	 * @brief Load a new plugin.
	 * 
	 * @param path Path to the Shared Object
	 */
	explicit plugin_loader(const std::string& path);
	
	plugin_loader(const plugin_loader& other) = delete;
	plugin_loader(plugin_loader&& other) = default;
	~plugin_loader() = default;

	plugin_loader& operator=(const plugin_loader& other) = delete;
	plugin_loader& operator=(plugin_loader&& other) = default;

	/**
	 * @brief Get the loaded plugin.
	 * 
	 * May return nullptr if is_open() is false.
	 * 
	 * @return const plugin* The loaded plugin.
	 */
	const plugin* get_plugin() const noexcept;

	/**
	 * @brief Check if the shared object is loaded.
	 * 
	 * @return true The shared object is loaded.
	 * @return false The shared object is not loaded.
	 */
	bool is_open() const noexcept;

	/**
	 * @brief Unloads the plugin. 
	 * 
	 * This method should be used with care as it does not handle
	 * deregister the plugin.
	 * 
	 */
	void reset();

	/**
	 * @brief Loads a new plugin.
	 * 
	 * If a plugin is already loaded, it will unload it. Use it with caution.
	 * 
	 * @param path Path to the Shared Object to be loaded.
	 */
	void load(const std::string& path);

private:
	dynamic_library m_dynamic_library;
	const plugin* m_plugin;
};

} // namespace rex
