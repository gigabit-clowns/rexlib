// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "version.hpp"
#include "platform/dynamic_shared_object.h"

#include <string>

namespace xmipp4
{

class service_catalog;

/**
 * @brief Abstract class representing a plugin interface.
 * 
 * A concrete instance of this class may be used to connect a plugin
 * to the core.
 * 
 */
class REXLIB_API plugin
{
public:
	plugin() = default;
	plugin(const plugin& other) = default;
	plugin(plugin&& other) = default;
	virtual ~plugin() = default;

	plugin& operator=(const plugin& other) = default;
	plugin& operator=(plugin&& other) = default;

	/**
	 * @brief Get the name of the plugin.
	 * 
	 * @return const std::string& The name.
	 */
	virtual const std::string& get_name() const noexcept = 0;

	/**
	 * @brief Get the version of the plugin.
	 * 
	 * @return version The version.
	 */
	virtual version get_version() const noexcept = 0;

	/**
	 * @brief Register this plugin at a given interface catalog.
	 * 
	 * @param catalog The catalog where this plugin will be registered.
	 */
	virtual void register_at(service_catalog& catalog) const = 0;
};

} // namespace xmipp4
