// SPDX-License-Identifier: GPL-3.0-only

#include "communicator.hpp"

#include <xmipp4/core/communication/communicator.hpp>

namespace xmipp4
{
namespace communication
{

namespace py = pybind11;

void bind_communicator(pybind11::module_ &m)
{
    py::class_<communicator, std::shared_ptr<communicator>>(m, "Communicator")
        .def_property_readonly("rank", &communicator::get_rank)
        .def_property_readonly("size", &communicator::get_size)
        .def(
            "split", 
            &communicator::split, 
            py::arg("color"), 
            py::arg("key"),
            py::keep_alive<0, 1>()
        )
        .def("barrier", &communicator::barrier);

}

} // namespace communication
} // namespace xmipp4
