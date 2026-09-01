// SPDX-License-Identifier: GPL-3.0-only

#include "array_descriptor.hpp"

#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/span.hpp>

#include <pybind11/stl.h> // Required for std::vector binding
#include <pybind11/operators.h>

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

array_descriptor_class declare_array_descriptor(pybind11::module_ &m)
{
	return array_descriptor_class(m, "ArrayDescriptor");
}

void define_array_descriptor(array_descriptor_class &c, pybind11::module_ &m)
{
	c
		.def(py::init<>())
		.def_property_readonly("data_type", &array_descriptor::get_data_type)
		.def(py::self == py::self)
		.def(py::self != py::self);

	m.def(
		"make_contiguous_array_descriptor",
		&make_contiguous_array_descriptor_from_vector,
		py::arg("extents"), py::arg("data_type")
	);
	m.def("is_initialized", &is_initialized);
}


} // namespace rexlib
