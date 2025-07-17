// SPDX-License-Identifier: GPL-3.0-only

#include "device_index.hpp"

#include <xmipp4/core/compute/device_index.hpp>

#include <pybind11/operators.h>

#include <sstream>

namespace xmipp4
{
namespace compute
{

namespace py = pybind11;

static std::string to_string(const device_index &l)
{
    std::ostringstream oss;
    oss << l;
    return oss.str();
}

static std::string to_repr(const device_index &l)
{
    std::ostringstream oss;
    oss << "DeviceIndex(backend=\"" << l.get_backend_name() << "\", " 
        << "id=" << l.get_device_id() << ")";
    return oss.str();
}

static device_index from_string(const std::string &str)
{
    device_index result;
    if (!parse_device_index(str, result))
    {
        std::ostringstream oss;
        oss << "Invalid device_index syntax \"" << str << "\"\n"
            << "Expected syntax \"backend_name:device_id\" or \"backend_name\"";
        throw std::invalid_argument(oss.str());
    }
    return result;
}



void bind_device_index(pybind11::module_ &m)
{
    py::class_<device_index>(m, "DeviceIndex")
        .def(py::init<py::str, py::size_t>(), py::arg("backend"), py::arg("id"))
        .def(py::init(&from_string))
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(py::self < py::self)
        .def(py::self <= py::self)
        .def(py::self > py::self)
        .def(py::self >= py::self)
        .def("__str__", &to_string)
        .def("__repr__", &to_repr)
        .def_property_readonly("backend", &device_index::get_backend_name)
        .def_property_readonly("id", &device_index::get_device_id)
        .def(py::pickle(
            [](const device_index &l) -> pybind11::tuple // __getstate__
            {
                return py::make_tuple(
                    l.get_backend_name(),
                    l.get_device_id()
                );
            },
            [](py::tuple t) -> device_index  // __setstate__
            {
                return device_index(
                    t[0].cast<std::string>(),
                    t[1].cast<std::size_t>()
                );
            }
        ));

}

} // namespace compute
} // namespace xmipp4
