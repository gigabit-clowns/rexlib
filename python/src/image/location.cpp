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

#include "location.hpp"

#include <xmipp4/core/image/location.hpp>

#include <pybind11/operators.h>

#include <sstream>

namespace xmipp4
{
namespace image
{

namespace py = pybind11;



static std::string location_to_string(const location &l)
{
    std::ostringstream oss;
    oss << l;
    return oss.str();
}



void register_version(pybind11::module_ &m)
{
    py::class_<location>(m, "version")
        .def(py::init<py::str, py::int_>())
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(py::self < py::self)
        .def(py::self <= py::self)
        .def(py::self > py::self)
        .def(py::self >= py::self)
        .def("__str__", &location_to_string)
        .def_property("filename", &location::get_filename, &location::set_filename<std::string>)
        .def_property("position", &location::get_position, &location::set_position)
        .def(py::pickle(
            [](const location &l) -> pybind11::tuple // __getstate__
            {
                return py::make_tuple(
                    l.get_filename(), 
                    l.get_position() 
                );
            },
            [](py::tuple t) -> location  // __setstate__
            {
                return location(
                    t[0].cast<std::string>(),
                    t[1].cast<int>()
                );
            }
        ));

    m.def("has_position", &has_position);
    m.def("is_contiguous", &is_contiguous);
    m.def("parse_location", &parse_location);
}

} // namespace image
} // namespace xmipp4
