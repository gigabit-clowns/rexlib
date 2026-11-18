// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/system/dynamic_library.hpp>

namespace rexlib
{

class plugin;

/**
 * @brief Helper class to load plugins from shared objects.
 *
 * RAII style: it unloads the shared object when destroyed, unless it was
 * released.
 *
 */
class plugin_loader
{
public:
	plugin_loader();

	/**
	 * @brief Load a new plugin.
	 *
	 * The plugin is asked for its plugin_compatibility first, and refused
	 * unless it can run with this library.
	 *
	 * @param path Path to the Shared Object, in UTF-8
	 * @throws dynamic_library_error If the shared object cannot be loaded
	 * @throws plugin_load_error If it is not a plugin this library can run
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
	 * @brief Keeps the shared object loaded for the rest of the process.
	 *
	 * The plugin, and whatever its code creates, stays valid however long
	 * this object lives. is_open() returns false afterwards.
	 */
	void release() noexcept;

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

} // namespace rexlib
