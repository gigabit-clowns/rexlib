// SPDX-License-Identifier: GPL-3.0-only

#include "plugin_manager.hpp"

#include <xmipp4/core/plugin_manager.hpp>

#include <xmipp4/core/plugin.hpp>

namespace xmipp4
{

namespace py = pybind11;

void bind_plugin_manager(pybind11::module_ &m)
{
    py::class_<plugin_manager>(m, "PluginManager")
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
            [](plugin_manager &manager, const std::string &directory) {
                discover_plugins(directory, manager);
            },
            py::arg("directory") = get_plugin_directory()
        )
        .def_property_readonly(
            "plugins",
            [](const py::object &self) -> py::list
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
