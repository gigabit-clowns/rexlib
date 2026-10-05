// SPDX-License-Identifier: GPL-3.0-only

#include "image_descriptor.hpp"

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

namespace rexlib
{

namespace py = pybind11;

static py::tuple to_tuple(span<const std::size_t> extents)
{
	py::tuple result(extents.size());
	for (std::size_t i = 0; i < extents.size(); ++i)
	{
		result[i] = extents[i];
	}
	return result;
}

static em::image_descriptor py_init(
	const std::vector<std::size_t> &extents,
	std::size_t core_rank,
	numerical_type data_type
)
{
	return em::image_descriptor(make_span(extents), core_rank, data_type);
}

static py::tuple get_extents(const em::image_descriptor &self)
{
	return to_tuple(self.get_extents());
}

static py::tuple get_core_extents(const em::image_descriptor &descriptor)
{
	return to_tuple(em::get_core_extents(descriptor));
}

static std::string to_repr(const em::image_descriptor &self)
{
	std::ostringstream oss;
	oss << "ImageDescriptor(extents="
		<< py::repr(get_extents(self)).cast<std::string>()
		<< ", core_rank=" << self.get_core_rank()
		<< ", data_type="
		<< py::str(py::cast(self.get_data_type())).cast<std::string>() << ")";
	return oss.str();
}

image_descriptor_class declare_image_descriptor(pybind11::module_ &m)
{
	return image_descriptor_class(m, "ImageDescriptor");
}

void define_image_descriptor(
	image_descriptor_class &c,
	pybind11::module_ &m
)
{
	c
		.def(
			py::init(&py_init),
			py::arg("extents"), py::arg("core_rank"), py::arg("data_type")
		)
		.def(py::self == py::self)
		.def(py::self != py::self)
		.def("__hash__", &em::image_descriptor::hash)
		.def("__repr__", &to_repr)
		.def_property_readonly("extents", &get_extents)
		.def_property_readonly(
			"core_rank",
			&em::image_descriptor::get_core_rank
		)
		.def_property_readonly(
			"data_type",
			&em::image_descriptor::get_data_type
		);

	m.def("get_core_extents", &get_core_extents, py::arg("descriptor"));
}

} // namespace rexlib
