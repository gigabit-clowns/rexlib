// SPDX-License-Identifier: GPL-3.0-only

#include "image_location.hpp"

#include <rexlib/em/image/image_location.hpp>

#include <pybind11/operators.h>

#include <sstream>
#include <stdexcept>
#include <string>

namespace rexlib
{

namespace py = pybind11;

static std::string to_string(const em::image_location &l)
{
	return em::to_string(l);
}

static std::string to_repr(const em::image_location &l)
{
	std::ostringstream oss;
	oss << "ImageLocation(path=\"" << l.get_path() << "\", "
		<< "position=" << l.get_position_in_stack() << ")";
	return oss.str();
}

static em::image_location from_string(const std::string &text)
{
	em::image_location result;
	if (!em::parse_image_location(text, result))
	{
		std::ostringstream oss;
		oss << "Invalid image_location syntax \"" << text << "\"\n"
			<< R"(Expected syntax "index@path", one based, or "path")";
		throw std::invalid_argument(oss.str());
	}
	return result;
}

image_location_class declare_image_location(pybind11::module_ &m)
{
	return image_location_class(m, "ImageLocation");
}

void define_image_location(image_location_class &c)
{
	c
		.def(
			py::init<std::string, std::size_t>(),
			py::arg("path"),
			py::arg("position") = em::image_location::no_position
		)
		.def(py::init<>())
		.def_static("from_string", &from_string, py::arg("text"))
		.def(py::self == py::self)
		.def(py::self != py::self)
		.def(py::self < py::self)
		.def(py::self <= py::self)
		.def(py::self > py::self)
		.def(py::self >= py::self)
		.def("__hash__", &em::image_location::hash)
		.def("__str__", &to_string)
		.def("__repr__", &to_repr)
		.def_property_readonly("path", &em::image_location::get_path)
		.def_property_readonly(
			"position_in_stack",
			&em::image_location::get_position_in_stack
		)
		.def(py::pickle(
			[](const em::image_location &l) // __getstate__
			{
				return py::make_tuple(
					l.get_path(),
					l.get_position_in_stack()
				);
			},
			[](py::tuple t)  // __setstate__
			{
				return em::image_location(
					t[0].cast<std::string>(),
					t[1].cast<std::size_t>()
				);
			}
		));

	c.attr("no_position") = em::image_location::no_position;
}

} // namespace rexlib
