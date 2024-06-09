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

#include "version.hpp"

#include <xmipp4/core/version.hpp>

#include <pybind11/operators.h>

namespace xmipp4
{

namespace py = pybind11;

void register_version(pybind11::module_ &m)
{
    py::class_<version>(m, "version")
        .def(py::init<py::int_, py::int_, py::int_>())
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(py::self < py::self)
        .def(py::self <= py::self)
        .def(py::self > py::self)
        .def(py::self >= py::self)
        .def_property("major", &version::get_major, &version::set_major)
        .def_property("minor", &version::get_minor, &version::set_minor)
        .def_property("patch", &version::get_patch, &version::set_patch)
        .def(py::pickle(
            [](const version &v) // __getstate__
            {
                return py::make_tuple(
                    v.get_major(), 
                    v.get_minor(), 
                    v.get_patch()
                );
            },
            [](py::tuple t) -> version  // __setstate__
            {
                return version(
                    t[0].cast<int>(),
                    t[1].cast<int>(),
                    t[2].cast<int>()
                );
            }
        ));
}

} // namespace xmipp4
