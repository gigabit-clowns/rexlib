// SPDX-License-Identifier: GPL-3.0-only

#include "interface_catalog.hpp"

#include <xmipp4/core/interface_catalog.hpp>

#include <xmipp4/core/plugin_manager.hpp>

namespace xmipp4
{

namespace py = pybind11;

void bind_interface_catalog(pybind11::module_ &m)
{
    py::class_<interface_catalog>(m, "InterfaceCatalog")
        .def(py::init<bool>(), py::arg("register_builtin_backends") = true)
        .def(
            "register_plugins", 
            [](interface_catalog &catalog, const plugin_manager &manager) 
            {
                return register_all_plugins_at(manager, catalog);
            },
            py::keep_alive<1, 2>() // Do not destroy the manager before the catalog
        );

}

} // namespace xmipp4
