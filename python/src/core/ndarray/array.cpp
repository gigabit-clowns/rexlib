// SPDX-License-Identifier: GPL-3.0-only

#include "array.hpp"

#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/ndarray/host_access.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>

#include <pybind11/complex.h>
#include <pybind11/stl.h> // Required for std::vector binding

#include <complex>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
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

static std::string get_buffer_format(numerical_type type)
{
	switch (type)
	{
	case numerical_type::boolean:
		return py::format_descriptor<bool>::format();
	case numerical_type::char8:
		return py::format_descriptor<char>::format();
	case numerical_type::int8:
		return py::format_descriptor<std::int8_t>::format();
	case numerical_type::uint8:
		return py::format_descriptor<std::uint8_t>::format();
	case numerical_type::int16:
		return py::format_descriptor<std::int16_t>::format();
	case numerical_type::uint16:
		return py::format_descriptor<std::uint16_t>::format();
	case numerical_type::int32:
		return py::format_descriptor<std::int32_t>::format();
	case numerical_type::uint32:
		return py::format_descriptor<std::uint32_t>::format();
	case numerical_type::int64:
		return py::format_descriptor<std::int64_t>::format();
	case numerical_type::uint64:
		return py::format_descriptor<std::uint64_t>::format();
	case numerical_type::float16:
		return "e";
	case numerical_type::float32:
		return py::format_descriptor<float>::format();
	case numerical_type::float64:
		return py::format_descriptor<double>::format();
	case numerical_type::complex_float32:
		return py::format_descriptor<std::complex<float>>::format();
	case numerical_type::complex_float64:
		return py::format_descriptor<std::complex<double>>::format();
	default:
		break;
	}

	std::ostringstream oss;
	oss << "The buffer protocol has no format for the data type " << type
		<< ".";
	throw py::buffer_error(oss.str());
}

static py::buffer_info get_buffer(array &self)
{
	void *data = nullptr;
	{
		const py::gil_scoped_release release;
		data = get_host_data(array_ref(self));
	}

	const auto &descriptor = self.get_descriptor();
	const auto &layout = descriptor.get_layout();
	const auto data_type = descriptor.get_data_type();
	const auto item_size = static_cast<py::ssize_t>(get_size(data_type));

	std::vector<std::ptrdiff_t> strides;
	layout.get_strides(strides);

	std::vector<py::ssize_t> shape;
	for (const auto extent : get_extents(self))
	{
		shape.push_back(static_cast<py::ssize_t>(extent));
	}

	std::vector<py::ssize_t> byte_strides;
	for (const auto stride : strides)
	{
		byte_strides.push_back(stride * item_size);
	}

	const auto rank = static_cast<py::ssize_t>(shape.size());
	auto *first = static_cast<char*>(data) + layout.get_offset() * item_size;
	return py::buffer_info(
		first,
		item_size,
		get_buffer_format(data_type),
		rank,
		std::move(shape),
		std::move(byte_strides)
	);
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
	return array_class(m, "Array", py::buffer_protocol());
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
		.def("__repr__", &to_repr)
		.def_buffer(&get_buffer);
}

} // namespace rexlib
