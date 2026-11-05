// SPDX-License-Identifier: GPL-3.0-only

#include "array.hpp"

#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/ndarray/host_access.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>

#include <nanobind/stl/string.h>

#include <array>
#include <cstddef>
#include <exception>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace rexlib
{

namespace nb = nanobind;

namespace
{

/**
 * What a Py_buffer handed out points to besides the memory itself. Owned by
 * the view through its `internal` field.
 */
struct buffer_view
{
	std::vector<Py_ssize_t> shape;
	std::vector<Py_ssize_t> strides;
	std::string format;
};

std::vector<std::size_t> get_extents(const array &self)
{
	std::vector<std::size_t> extents;
	self.get_descriptor().get_layout().get_extents(extents);
	return extents;
}

nb::tuple get_shape(const array &self)
{
	const auto extents = get_extents(self);
	nb::tuple_builder shape(extents.size());
	for (const auto extent : extents)
	{
		shape.put(extent);
	}
	return shape.commit();
}

numerical_type get_data_type(const array &self)
{
	return self.get_descriptor().get_data_type();
}

std::size_t py_len(const array &self)
{
	const auto extents = get_extents(self);
	if (extents.empty())
	{
		throw nb::type_error("len() of an array with no dimensions");
	}
	return extents.front();
}

const char* get_buffer_format(numerical_type type) noexcept
{
	switch (type)
	{
	case numerical_type::boolean:
		return "?";
	case numerical_type::char8:
		return std::is_signed<char>::value ? "b" : "B";
	case numerical_type::int8:
		return "b";
	case numerical_type::uint8:
		return "B";
	case numerical_type::int16:
		return "h";
	case numerical_type::uint16:
		return "H";
	case numerical_type::int32:
		return "i";
	case numerical_type::uint32:
		return "I";
	case numerical_type::int64:
		return "q";
	case numerical_type::uint64:
		return "Q";
	case numerical_type::float16:
		return "e";
	case numerical_type::float32:
		return "f";
	case numerical_type::float64:
		return "d";
	case numerical_type::complex_float32:
		return "Zf";
	case numerical_type::complex_float64:
		return "Zd";
	default:
		return nullptr;
	}
}

bool is_contiguous(
	const std::vector<Py_ssize_t> &shape,
	const std::vector<Py_ssize_t> &strides,
	Py_ssize_t item_size,
	bool fortran
) noexcept
{
	Py_ssize_t expected = item_size;
	const auto rank = shape.size();
	for (std::size_t i = 0; i < rank; ++i)
	{
		const auto axis = fortran ? i : rank - 1 - i;
		if (shape[axis] == 0)
		{
			return true;
		}
		if (shape[axis] != 1 && strides[axis] != expected)
		{
			return false;
		}
		expected *= shape[axis];
	}
	return true;
}

void fill_buffer(PyObject *self, Py_buffer *view, int flags)
{
	auto &source = *nb::inst_ptr<array>(self);

	const auto &descriptor = source.get_descriptor();
	const auto data_type = descriptor.get_data_type();
	const auto *format = get_buffer_format(data_type);
	if (format == nullptr)
	{
		std::ostringstream oss;
		oss << "The buffer protocol has no format for the data type "
			<< data_type << ".";
		throw nb::buffer_error(oss.str().c_str());
	}

	void *data = nullptr;
	{
		const nb::gil_scoped_release release;
		data = get_host_data(array_ref(source));
	}

	const auto &layout = descriptor.get_layout();
	const auto item_size = static_cast<Py_ssize_t>(get_size(data_type));

	auto result = std::make_unique<buffer_view>();
	result->format = format;

	std::vector<std::ptrdiff_t> strides;
	layout.get_strides(strides);
	Py_ssize_t count = 1;
	for (const auto extent : get_extents(source))
	{
		result->shape.push_back(static_cast<Py_ssize_t>(extent));
		count *= static_cast<Py_ssize_t>(extent);
	}
	for (const auto stride : strides)
	{
		result->strides.push_back(stride * item_size);
	}

	const auto c_contiguous =
		is_contiguous(result->shape, result->strides, item_size, false);
	const auto f_contiguous =
		is_contiguous(result->shape, result->strides, item_size, true);
	const auto strided = (flags & PyBUF_STRIDES) == PyBUF_STRIDES;
	if (
		(!strided && !c_contiguous) ||
		((flags & PyBUF_C_CONTIGUOUS) == PyBUF_C_CONTIGUOUS &&
			!c_contiguous) ||
		((flags & PyBUF_F_CONTIGUOUS) == PyBUF_F_CONTIGUOUS &&
			!f_contiguous) ||
		((flags & PyBUF_ANY_CONTIGUOUS) == PyBUF_ANY_CONTIGUOUS &&
			!c_contiguous && !f_contiguous)
	)
	{
		throw nb::buffer_error(
			"The array is not laid out as the consumer requires."
		);
	}

	view->buf = static_cast<char*>(data) + layout.get_offset() * item_size;
	view->obj = Py_NewRef(self);
	view->len = count * item_size;
	view->readonly = 0;
	view->itemsize = item_size;
	view->format =
		(flags & PyBUF_FORMAT) == PyBUF_FORMAT
		? result->format.data()
		: nullptr;
	view->ndim = static_cast<int>(result->shape.size());
	view->shape =
		(flags & PyBUF_ND) == PyBUF_ND ? result->shape.data() : nullptr;
	view->strides = strided ? result->strides.data() : nullptr;
	view->suboffsets = nullptr;
	view->internal = result.release();
}

int get_buffer(PyObject *self, Py_buffer *view, int flags) noexcept
{
	view->obj = nullptr;
	try
	{
		fill_buffer(self, view, flags);
		return 0;
	}
	catch (nb::python_error &error)
	{
		error.restore();
	}
	catch (const nb::builtin_exception &error)
	{
		PyErr_SetString(PyExc_BufferError, error.what());
	}
	catch (const std::exception &error)
	{
		PyErr_SetString(PyExc_BufferError, error.what());
	}
	return -1;
}

void release_buffer(PyObject* /*self*/, Py_buffer *view) noexcept
{
	const std::unique_ptr<buffer_view> owned(
		static_cast<buffer_view*>(view->internal)
	);
}

const std::array<PyType_Slot, 3> array_slots = {{
	{ Py_bf_getbuffer, reinterpret_cast<void*>(&get_buffer) },
	{ Py_bf_releasebuffer, reinterpret_cast<void*>(&release_buffer) },
	{ 0, nullptr }
}};

std::string to_repr(const array &self)
{
	std::ostringstream oss;
	oss << "Array(descriptor="
		<< nb::str(nb::cast(self.get_descriptor())).c_str() << ")";
	return oss.str();
}

} // anonymous namespace

array_class declare_array(nb::module_ &m)
{
	return array_class(m, "Array", nb::type_slots(array_slots.data()));
}

void define_array(array_class &c)
{
	c
		.def_prop_ro(
			"descriptor",
			&array::get_descriptor,
			nb::rv_policy::reference_internal
		)
		.def_prop_ro("shape", &get_shape)
		.def_prop_ro("data_type", &get_data_type)
		.def("__len__", &py_len)
		.def("__repr__", &to_repr);
}

} // namespace rexlib
