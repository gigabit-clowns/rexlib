// SPDX-License-Identifier: GPL-3.0-only

#include "communicator_manager.hpp"

#include <xmipp4/core/communication/communicator_manager.hpp>

#include <xmipp4/core/service_catalog.hpp>
#include <xmipp4/core/communication/communicator.hpp>

#include <pybind11/stl.h> // Required for std::vector binding

namespace xmipp4
{
namespace communication
{

static communicator_manager& get_communicator_manager(service_catalog &catalog)
{
    return catalog.get_service_manager<communicator_manager>();
}



namespace py = pybind11;

void bind_communicator_manager(pybind11::module_ &m)
{
    py::class_<communicator_manager>(m, "CommunicatorManager")
        .def_property_readonly(
            "backends",
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
            py::arg("name"),
            py::keep_alive<0, 1>()
        )
        .def(
            "create_preferred_world_communicator", 
            &communicator_manager::create_preferred_world_communicator,
            py::keep_alive<0, 1>()
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
