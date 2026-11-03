// SPDX-License-Identifier: GPL-3.0-only

#include "plugin_manager.hpp"

#include <rexlib/core/plugin_manager.hpp>

#include <rexlib/core/plugin.hpp>

#include <nanobind/stl/optional.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include <optional>
#include <string>

namespace rexlib
{

namespace nb = nanobind;

plugin_manager_class declare_plugin_manager(nb::module_ &m)
{
	return plugin_manager_class(m, "PluginManager");
}

void define_plugin_manager(plugin_manager_class &c, nb::module_ &m)
{
	c
		.def(nb::init<>())
		.def(
			"add_plugin", 
			&plugin_manager::add_plugin, 
			nb::arg("plugin"),
			nb::keep_alive<1, 2>() //Don't destroy the plugin before the manager
		)
		.def(
			"load_plugin", 
			&plugin_manager::load_plugin, 
			nb::arg("path"), 
			nb::rv_policy::reference_internal
		)
		.def(
			"discover_plugins",
			[](
				plugin_manager &manager,
				const std::optional<std::string> &directory
			) {
				if (directory)
				{
					discover_plugins(*directory, manager);
				}
				else
				{
					// Walks the whole search path rather than just its
					// first entry.
					discover_plugins(manager);
				}
			},
			nb::arg("directory") = nb::none()
		)
		.def_prop_ro(
			"plugins",
			[](nb::handle self)
			{
				nb::list result;

				const auto &manager = nb::cast<const plugin_manager&>(self);
				const auto count = manager.get_plugin_count();
				for (std::size_t i = 0; i < count; ++i)
				{
					auto object = nb::cast(
						manager.get_plugin(i), 
						nb::rv_policy::reference_internal,
						self
					);
					result.append(std::move(object));
				}

				return result;
			}
		);

	m.def("get_plugin_search_path", &get_plugin_search_path);
	m.def("get_default_plugin_directory", &get_default_plugin_directory);
}

} // namespace rexlib
