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
