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

#include "communicator_manager.hpp"

#include <xmipp4/core/communication/communicator_manager.hpp>

#include <xmipp4/core/interface_registry.hpp>
#include <xmipp4/core/communication/communicator.hpp>

#include <pybind11/stl.h> // Required for std::vector binding

namespace xmipp4
{
namespace communication
{

static communicator_manager& get_communicator_manager(interface_registry &registry)
{
    return registry.get_interface_manager<communicator_manager>();
}



namespace py = pybind11;

void bind_communicator_manager(pybind11::module_ &m)
{
    py::class_<communicator_manager>(m, "CommunicatorManager")
        .def(
            "enumerate_backends",
            [] (const communicator_manager &manager) -> std::vector<std::string>
            {
                std::vector<std::string> backends;
                manager.enumerate_backends(backends);
                return backends;
            }
        )
        .def(
            "create_world_communicator", 
            &communicator_manager::create_world_communicator, 
            py::arg("name")
        )
        .def(
            "create_preferred_world_communicator", 
            &communicator_manager::create_preferred_world_communicator
        );

    m.def(
        "get_communicator_manager",
        &get_communicator_manager,
        py::return_value_policy::reference,
        py::keep_alive<0, 1>()
    );

}

} // namespace communication
} // namespace xmipp4
