// SPDX-License-Identifier: GPL-3.0-only

#include "plugin_manager.hpp"

#include <rexlib/core/plugin_manager.hpp>

#include <rexlib/core/plugin.hpp>

#include <pybind11/stl.h>

#include <optional>
#include <string>

namespace rexlib
{

namespace py = pybind11;

plugin_manager_class declare_plugin_manager(pybind11::module_ &m)
{
	return plugin_manager_class(m, "PluginManager");
}

void define_plugin_manager(plugin_manager_class &c, pybind11::module_ &m)
{
	c
		.def(py::init<>())
		.def(
			"add_plugin", 
			&plugin_manager::add_plugin, 
			py::arg("plugin"),
			py::keep_alive<1, 2>() //Don't destroy the plugin before the manager
		)
		.def(
			"load_plugin", 
			&plugin_manager::load_plugin, 
			py::arg("path"), 
			py::return_value_policy::reference_internal
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
			py::arg("directory") = py::none()
		)
		.def_property_readonly(
			"plugins",
			[](const py::object &self)
			{
				py::list result;
				
				const auto &manager = self.cast<const plugin_manager&>();
				const auto count = manager.get_plugin_count();
				for (std::size_t i = 0; i < count; ++i)
				{
					auto object = py::cast(
						manager.get_plugin(i), 
						py::return_value_policy::reference_internal,
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
