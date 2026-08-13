// SPDX-License-Identifier: GPL-3.0-only

#include "plugin_manager.hpp"

#include <xmipp4/core/plugin_manager.hpp>

#include <xmipp4/core/plugin.hpp>

#include <pybind11/stl.h>

#include <optional>
#include <string>

namespace xmipp4
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
				discover_plugins(
					directory.value_or(get_plugin_directory()),
					manager
				);
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

	m.def("get_plugin_directory", &get_plugin_directory);
	m.def("get_default_plugin_directory", &get_default_plugin_directory);
}

} // namespace xmipp4
