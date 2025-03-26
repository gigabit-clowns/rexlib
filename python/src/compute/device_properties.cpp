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
