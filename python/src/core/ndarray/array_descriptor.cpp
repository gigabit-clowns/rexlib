// SPDX-License-Identifier: GPL-3.0-only

#include "array_descriptor.hpp"

#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/span.hpp>

#include <pybind11/stl.h> // Required for std::vector binding
#include <pybind11/operators.h>

#include <sstream>
#include <vector>

namespace rexlib
{

namespace py = pybind11;

static array_descriptor make_contiguous_array_descriptor_from_vector(
	const std::vector<std::size_t> &extents,
	numerical_type data_type
)
{
	return make_contiguous_array_descriptor(make_span(extents), data_type);
}

static std::vector<std::size_t> get_extents(const array_descriptor &self)
{
	std::vector<std::size_t> extents;
	self.get_layout().get_extents(extents);
	return extents;
}

static py::tuple get_shape(const array_descriptor &self)
{
	const auto extents = get_extents(self);
	py::tuple shape(extents.size());
	for (std::size_t i = 0; i < extents.size(); ++i)
	{
		shape[i] = extents[i];
	}
	return shape;
}

static std::string to_repr(const array_descriptor &self)
{
	if (!is_initialized(self))
	{
		return "ArrayDescriptor()";
	}

	std::ostringstream oss;
	oss << "ArrayDescriptor(shape=(";
	const auto extents = get_extents(self);
	for (std::size_t i = 0; i < extents.size(); ++i)
	{
		oss << extents[i] << (i + 1 < extents.size() ? ", " : "");
	}
	oss << (extents.size() == 1 ? ",)" : ")") << ", data_type="
		<< py::str(py::cast(self.get_data_type())).cast<std::string>() << ")";
	return oss.str();
}

array_descriptor_class declare_array_descriptor(pybind11::module_ &m)
{
	return array_descriptor_class(m, "ArrayDescriptor");
}

void define_array_descriptor(array_descriptor_class &c, pybind11::module_ &m)
{
	c
		.def(py::init<>())
		.def_property_readonly("data_type", &array_descriptor::get_data_type)
		.def_property_readonly("shape", &get_shape)
		.def(py::self == py::self)
		.def(py::self != py::self)
		.def("__hash__", &array_descriptor::hash)
		.def("__repr__", &to_repr);

	m.def(
		"make_contiguous_array_descriptor",
		&make_contiguous_array_descriptor_from_vector,
		py::arg("extents"), py::arg("data_type")
	);
	m.def("is_initialized", &is_initialized);
}


} // namespace rexlib
