// SPDX-License-Identifier: GPL-3.0-only

#include "index_table.hpp"

#include <rexlib/core/span.hpp>

#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace rexlib
{

namespace py = pybind11;

static void py_add(index_table &self, const std::vector<std::size_t> &index)
{
	self.add(make_span(index));
}

static py::tuple py_get(const index_table &self, std::size_t position)
{
	if (position >= self.get_index_count())
	{
		throw py::index_error("IndexTable index out of range");
	}

	const auto index = self.get(position);
	py::tuple result(index.size());
	for (std::size_t i = 0; i < index.size(); ++i)
	{
		result[i] = index[i];
	}
	return result;
}

// rexlib's indices are unsigned, so a negative one would wrap around into
// a huge index on its way in rather than fail.
static void py_require_non_negative(const py::array &indices)
{
	const auto values = py::array_t<
		std::int64_t,
		py::array::c_style | py::array::forcecast
	>::ensure(indices);
	if (!values)
	{
		throw py::error_already_set();
	}

	const auto *first = values.data();
	const auto *last = first + values.size();
	const auto is_negative = [] (std::int64_t value) { return value < 0; };
	if (std::any_of(first, last, is_negative))
	{
		throw py::value_error(
			"IndexTable.from_array takes no negative index"
		);
	}
}

// The rows are copied here rather than added one by one from Python, which
// would cross into C++ once per index.
static index_table py_from_array(const py::array &indices)
{
	const auto kind = indices.dtype().kind();
	if (kind != 'i' && kind != 'u')
	{
		throw py::type_error(
			"IndexTable.from_array takes an array of integers"
		);
	}
	if (indices.ndim() != 2)
	{
		throw py::value_error(
			"IndexTable.from_array takes an array of two dimensions, one "
			"row per index"
		);
	}
	if (kind == 'i')
	{
		py_require_non_negative(indices);
	}

	const auto values = py::array_t<
		std::size_t,
		py::array::c_style | py::array::forcecast
	>::ensure(indices);
	if (!values)
	{
		throw py::error_already_set();
	}

	const auto count = static_cast<std::size_t>(values.shape(0));
	const auto rank = static_cast<std::size_t>(values.shape(1));
	const auto *data = values.data();

	index_table result(rank);
	result.reserve(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		result.add(make_span(data + i * rank, rank));
	}
	return result;
}

// The array is a copy: a view would be left dangling by the next index
// added to the table.
static py::array py_to_array(
	const index_table &self,
	const std::optional<py::dtype> &dtype,
	std::optional<bool> copy
)
{
	if (copy.has_value() && !*copy)
	{
		throw py::value_error(
			"An IndexTable cannot become an array without a copy"
		);
	}

	const auto count = self.get_index_count();
	const auto rank = self.get_rank();
	py::array_t<std::size_t> result({
		static_cast<py::ssize_t>(count),
		static_cast<py::ssize_t>(rank)
	});

	auto *data = result.mutable_data();
	for (std::size_t i = 0; i < count; ++i)
	{
		const auto index = self.get(i);
		std::copy(index.begin(), index.end(), data + i * rank);
	}

	if (!dtype.has_value())
	{
		return result;
	}
	return result.attr("astype")(*dtype).cast<py::array>();
}

index_table_class declare_index_table(pybind11::module_ &m)
{
	return index_table_class(m, "IndexTable");
}

void define_index_table(index_table_class &c)
{
	c
		.def(py::init<>())
		.def(py::init<std::size_t>(), py::arg("rank"))
		.def_static("from_array", &py_from_array, py::arg("indices"))
		.def("add", &py_add, py::arg("index"))
		.def("clear", &index_table::clear)
		.def("reserve", &index_table::reserve, py::arg("count"))
		.def_property_readonly("rank", &index_table::get_rank)
		.def("__len__", &index_table::get_index_count)
		.def("__getitem__", &py_get, py::arg("position"))
		.def(
			"__array__", &py_to_array,
			py::arg("dtype") = py::none(), py::arg("copy") = py::none()
		);
}

} // namespace rexlib
