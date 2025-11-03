// SPDX-License-Identifier: GPL-3.0-only

#include "version.hpp"

#include <xmipp4/core/version.hpp>

#include <sstream>

#include <pybind11/operators.h>

namespace xmipp4
{

namespace py = pybind11;

static std::string to_string(const version &v)
{
	std::ostringstream oss;
	oss << v;
	return oss.str();
}

static std::string to_repr(const version &v)
{
	std::ostringstream oss;
	oss << "Version(major=" << v.get_major() << ", "
		<< "minor=" << v.get_minor() << ", "
		<< "patch=" << v.get_patch() << ")";
	return oss.str();
}

void bind_version(pybind11::module_ &m)
{
	py::class_<version>(m, "Version")
		.def(
			py::init<py::int_, py::int_, py::int_>(), 
			py::arg("major"), py::arg("minor"), py::arg("patch")
	)
		.def(py::self == py::self)
		.def(py::self != py::self)
		.def(py::self < py::self)
		.def(py::self <= py::self)
		.def(py::self > py::self)
		.def(py::self >= py::self)
		.def("__str__", &to_string)
		.def("__repr__", &to_repr)
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
