// SPDX-License-Identifier: GPL-3.0-only

#include "cast.hpp"

#include <rexlib/functional/cast.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>

#include <pybind11/stl.h>

#include <optional>

namespace rexlib
{

namespace py = pybind11;

static array py_cast_copy(
	const array &input,
	numerical_type target_type,
	const execution_context &context,
	std::optional<array*> out
)
{
	return rexlib::cast_copy(input, target_type, context, out.value_or(nullptr));
}

void bind_cast(pybind11::module_ &m)
{
	m.def(
		"cast",
		&rexlib::cast,
		py::arg("input"), py::arg("target_type"), py::arg("context")
	);
	m.def(
		"cast_copy", &py_cast_copy,
		py::arg("input"), py::arg("target_type"), py::arg("context"),
		py::arg("out") = py::none()
	);
}

} // namespace rexlib
