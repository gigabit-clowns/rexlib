// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "builtin_plugin_registry.hpp"

namespace rexlib
{

/**
 * @brief Add a plugin to a registry of built-in plugins upon construction.
 *
 * Declare one namespace-scope object of this type per built-in plugin, so
 * that the plugin adds itself during static initialization.
 *
 * @tparam Plugin The concrete plugin type. Must be default constructible.
 * @tparam Registry The registry to add to. Any type that takes a
 * @c std::unique_ptr<plugin> through @c add.
 */
template <typename Plugin, typename Registry = builtin_plugin_registry>
class builtin_plugin_registration
{
public:
	/**
	 * @brief Add a new @p Plugin to a registry.
	 *
	 * @param registry The registry to add to.
	 */
	explicit builtin_plugin_registration(Registry &registry);
};

} // namespace rexlib

#include "builtin_plugin_registration.inl"
