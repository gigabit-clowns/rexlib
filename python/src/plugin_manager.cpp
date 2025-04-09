/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

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
            [](const plugin_manager &manager) -> py::list
            {
                py::list result;
                
                const auto count = manager.get_plugin_count();
                for (std::size_t i = 0; i < count; ++i)
                {
                    auto object = py::cast(
                        manager.get_plugin(i), 
                        py::return_value_policy::reference
                        // FIXME keep alive the manager
                    );
                    result.append(std::move(object));
                }

                return result;
            },
            py::return_value_policy::reference_internal
        );

    m.def("get_plugin_directory", &get_plugin_directory);
    m.def("get_default_plugin_directory", &get_default_plugin_directory);
}

} // namespace xmipp4
