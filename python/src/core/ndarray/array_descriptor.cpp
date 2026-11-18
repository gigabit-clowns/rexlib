// SPDX-License-Identifier: GPL-3.0-only

#include "array_descriptor.hpp"

#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/span.hpp>

#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include <sstream>
#include <vector>

namespace rexlib
{

namespace nb = nanobind;

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

static nb::tuple get_shape(const array_descriptor &self)
{
	const auto extents = get_extents(self);
	nb::tuple_builder shape(extents.size());
	for (const auto extent : extents)
	{
		shape.put(extent);
	}
	return shape.commit();
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
		<< nb::str(nb::cast(self.get_data_type())).c_str() << ")";
	return oss.str();
}

array_descriptor_class declare_array_descriptor(nb::module_ &m)
{
	return array_descriptor_class(m, "ArrayDescriptor");
}

void define_array_descriptor(array_descriptor_class &c, nb::module_ &m)
{
	c
		.def(nb::init<>())
		.def_prop_ro("data_type", &array_descriptor::get_data_type)
		.def_prop_ro("shape", &get_shape)
		.def(nb::self == nb::self)
		.def(nb::self != nb::self)
		.def("__hash__", &array_descriptor::hash)
		.def("__repr__", &to_repr);

	m.def(
		"make_contiguous_array_descriptor",
		&make_contiguous_array_descriptor_from_vector,
		nb::arg("extents"), nb::arg("data_type")
	);
	m.def("is_initialized", &is_initialized);
}

} // namespace rexlib
