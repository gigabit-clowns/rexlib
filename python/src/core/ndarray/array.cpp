// SPDX-License-Identifier: GPL-3.0-only

#include "array.hpp"

#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/layout/strided_layout.hpp>

#include <pybind11/stl.h> // Required for std::vector binding

#include <sstream>
#include <vector>

namespace rexlib
{

namespace py = pybind11;

static std::vector<std::size_t> get_extents(const array &self)
{
	std::vector<std::size_t> extents;
	self.get_descriptor().get_layout().get_extents(extents);
	return extents;
}

static py::tuple get_shape(const array &self)
{
	const auto extents = get_extents(self);
	py::tuple shape(extents.size());
	for (std::size_t i = 0; i < extents.size(); ++i)
	{
		shape[i] = extents[i];
	}
	return shape;
}

static numerical_type get_data_type(const array &self)
{
	return self.get_descriptor().get_data_type();
}

static std::size_t py_len(const array &self)
{
	const auto extents = get_extents(self);
	if (extents.empty())
	{
		throw py::type_error("len() of an array with no dimensions");
	}
	return extents.front();
}

static std::string to_repr(const array &self)
{
	std::ostringstream oss;
	oss << "Array(descriptor="
		<< py::str(py::cast(self.get_descriptor())).cast<std::string>() << ")";
	return oss.str();
}

array_class declare_array(pybind11::module_ &m)
{
	return array_class(m, "Array");
}

void define_array(array_class &c)
{
	c
		.def_property_readonly(
			"descriptor",
			&array::get_descriptor,
			py::return_value_policy::reference_internal
		)
		.def_property_readonly("shape", &get_shape)
		.def_property_readonly("data_type", &get_data_type)
		.def("__len__", &py_len)
		.def("__repr__", &to_repr);
}

} // namespace rexlib
