// SPDX-License-Identifier: GPL-3.0-only

#include "device_properties.hpp"

#include <xmipp4/core/compute/device_properties.hpp>

namespace xmipp4
{
namespace compute
{

namespace py = pybind11;

void bind_device_properties(pybind11::module_ &m)
{
    py::class_<device_properties>(m, "DeviceProperties")
        .def(py::init<>())
        .def_property(
            "type", 
            &device_properties::get_type, 
            &device_properties::set_type
        )
        .def_property(
            "name", 
            &device_properties::get_name, 
            &device_properties::set_name<std::string>
        )
        .def_property(
            "physical_location",
            &device_properties::get_physical_location,
            &device_properties::set_physical_location<std::string> 
        )
        .def_property(
            "total_memory_bytes",
            &device_properties::get_total_memory_bytes,
            &device_properties::set_total_memory_bytes
        )   
        .def(py::pickle(
            [](const device_properties &l) -> pybind11::tuple // __getstate__
            {
                return py::make_tuple(
                    l.get_type(),
                    l.get_name(),
                    l.get_physical_location(),
                    l.get_total_memory_bytes()
                );
            },
            [](py::tuple t) -> device_properties  // __setstate__
            {
                device_properties result;
                result.set_type(t[0].cast<device_type>());
                result.set_name(t[1].cast<std::string>());
                result.set_physical_location(t[2].cast<std::string>());
                result.set_total_memory_bytes(t[3].cast<std::size_t>());
                return result;
            }
        ));

}

} // namespace compute
} // namespace xmipp4
