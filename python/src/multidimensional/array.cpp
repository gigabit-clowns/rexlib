// SPDX-License-Identifier: GPL-3.0-only

#include "array.hpp"

#include <xmipp4/core/multidimensional/array.hpp>

namespace xmipp4
{
namespace multidimensional
{

namespace py = pybind11;

static std::size_t compute_count(Py_ssize_t start, Py_ssize_t stop, Py_ssize_t step)
{
	if (stop == PY_SSIZE_T_MAX)
	{
		return end();
	}

	if (step < 0)
	{
		step = -step;
		std::swap(start, stop);
	}

	if (start > stop)
	{
		return 0;
	}

	return std::size_t((stop - start + step - 1) / step);
}

static slice convert_slice(py::slice s)
{
	auto *handle = s.ptr();
	Py_ssize_t start;
	Py_ssize_t stop;
	Py_ssize_t step;
	if (PySlice_Unpack(handle, &start, &stop, &step) < 0)
	{
		throw std::invalid_argument("Invalid slice");
	}
	if (stop < 0)
	{
		throw std::invalid_argument("Negative stop in slice not supported");
	}
	if (step == 0)
	{
		throw std::invalid_argument("Slice step cannot be zero");
	}

	return slice(start, compute_count(start, stop, step), step);
}

static dynamic_subscript convert_subscript(py::handle item)
{
	if (py::isinstance<py::int_>(item))
	{
		return dynamic_subscript(item.cast<std::ptrdiff_t>());
	}
	else if (py::isinstance<py::slice>(item))
	{
		auto s = py::reinterpret_borrow<py::slice>(item);
		return dynamic_subscript(convert_slice(s));
	}
	else if (item.is(py::none()))
	{
		return dynamic_subscript(new_axis_tag{});
	}
	else if (item.is(py::ellipsis()))
	{
		return dynamic_subscript(ellipsis_tag{});
	}
	else
	{
		throw std::invalid_argument("Invalid subscript type");
	}
}

static std::vector<dynamic_subscript> convert_subscripts(py::args args)
{
	std::vector<dynamic_subscript> subscripts;
	subscripts.reserve(args.size());
	for (auto item : args)
	{
		subscripts.emplace_back(convert_subscript(item));
	}
	return subscripts;
}

void bind_array(pybind11::module_ &m)
{
	py::class_<array>(m, "Array")
		.def_property_readonly("dtype", &array::get_data_type)
		.def_property_readonly("ndim", &array::get_rank)
		.def_property_readonly(
			"shape",
			[](const array &a)
			{
				std::vector<std::size_t> extents;
				a.get_extents(extents);
				return extents;
			}
		)
		.def(
			"__getitem__",
			[](array &a, py::args args)
			{
				auto subscripts = convert_subscripts(args);
				return a.apply_subscripts(make_span(subscripts));
			}
		)
		.def_property_readonly("T", &array::transpose)
		.def("transpose", &array::transpose)
		.def_property_readonly(
			"mT",
			[](array &a) { return a.matrix_transpose(-2, -1); }
		)
		.def(
			"matrix_transpose",
			&array::matrix_transpose,
			py::arg("axis1") = -2,
			py::arg("axis2") = -1
		)
		.def(
			"matrix_diagonal",
			&array::matrix_diagonal,
			py::arg("axis1") = -2,
			py::arg("axis2") = -1
		)
		.def("squeeze", &array::squeeze)
		.def("broadcast_to", &array::broadcast_to, py::arg("extents"));
}

} // namespace multidimensional
} // namespace xmipp4
